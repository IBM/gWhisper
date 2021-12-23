// Copyright 2019 IBM Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <libCli/Call.hpp>
#include <gRPC_utils/cli_call.h>
#include <google/protobuf/dynamic_message.h>
#include <libCli/OutputFormatting.hpp>
#include <libCli/ConnectionManager.hpp>
#include <libCli/MessageParsing.hpp>
#include "libCli/GrammarConstruction.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>

// for detecting if we are writing stdout to terminal or to pipe/file
#include <stdio.h>
#include <unistd.h>
#include <memory>

#include <libCli/cliUtils.hpp>

using namespace ArgParse;

static cli::OutputFormatterOptimizedForHumans::CustomStringModifier getModifier(ArgParse::ParsedElement &f_optionalModifier);

namespace cli
{
    /// Construct an OutputFormatter, which can be used to format protobuf messages.
    /// Depending on the given parseTree an OutputFormatter is selected.
    /// E.g. if user passed --jsonOutput, a formatter which is formatting messages
    /// to Json is created. Otherwise the default "Human Readable" formatter is
    /// created.
    /// @param parseTree CLI argument parse tree, which will be used to detemrine
    ///        which OputputFormatter to create.
    std::unique_ptr<OutputFormatter> createOutputFormatter(ParsedElement &parseTree)
    {
        if(parseTree.findFirstChild("JsonOutput") != "")
        {
            return std::make_unique<OutputFormatterJson>();
        }

        bool customOutputFormatRequested = false;
        auto ignored = parseTree.findFirstSubTree("CustomOutputFormat", customOutputFormatRequested);
        if(customOutputFormatRequested)
        {
            return std::make_unique<OutputFormatterCustom>(parseTree);
        }

        auto humanFormatter = std::make_unique<OutputFormatterOptimizedForHumans>();

        // disable colored output if explicitly specified:
        if (parseTree.findFirstChild("NoColor") != "")
        {
            humanFormatter->clearColorMap();
        }

        // disable map output as key => value if explicitly specified:
        if (parseTree.findFirstChild("NoSimpleMapOutput") != "")
        {
            humanFormatter->disableSimpleMapOutput();
        }

        // automatically disable colored output, when outputting to something
        // else than a terminal (pipes, files, etc.), except we explicitly
        // request color mode:
        if ((not isatty(fileno(stdout))) and (parseTree.findFirstChild("Color") == ""))
        {
            humanFormatter->clearColorMap();
        }

        return humanFormatter;
    }

    /// Construct a Message parser, which can be used to construct protobuf messages.
    /// Depending on the given parseTree a Parser is selected.
    /// E.g. depending on user given Option --jsonInput parser from JSOn is created.
    /// Otherwise parser from CLI args is created, which parses message fields
    /// directly from CLI arguments.
    /// @param parseTree CLI argument parse tree, which will be used to detemrine
    ///        which Parser to create.
    std::unique_ptr<MessageParser> createMessageParser(ParsedElement &parseTree)
    {
        if(parseTree.findFirstChild("JsonInput") != "")
        {
            return std::make_unique<MessageParserJson>();
        }
        else
        {
            return std::make_unique<MessageParserCli>();
        }
    }


    std::string getTimeString()
    {
        // unfortunately std::chrono::system_clock::to_time_t() is not available
        // with gcc4.8. So we use std::time and std::strftime instead.
        std::time_t t = std::time(0);
        char cstr[128];
        std::strftime(cstr, sizeof(cstr), "%Y-%m-%d %X", std::localtime(&t));
        return cstr;
    }

    int call(ParsedElement &parseTree)
    {
        std::string serviceName = parseTree.findFirstChild("Service");
        std::string methodName = parseTree.findFirstChild("Method");
        bool argsExist;
        ParsedElement &methodArgs = parseTree.findFirstSubTree("MethodArgs", argsExist);
        std::string serverAddress = cli::getServerUri(&parseTree);

        std::shared_ptr<grpc::Channel> channel = ConnectionManager::getInstance().getChannel(serverAddress, parseTree);

        if (not waitForChannelConnected(channel, getConnectTimeoutMs(&parseTree)))
        {
            std::cerr << "Error: channel connection attempt timed out" << std::endl;
            return -1;
        }

        const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, parseTree)->FindServiceByName(serviceName);
        if (service == nullptr)
        {
            std::cerr << "Error: Service '" << serviceName << "' not found" << std::endl;
            return -1;
        }

        auto method = service->FindMethodByName(methodName);
        if (method == nullptr)
        {
            std::cerr << "Error: Method not found" << std::endl;
            return -1;
        }

        const grpc::protobuf::Descriptor *inputType = method->input_type();
        // now we have to construct a protobuf from the parsed argument, which corresponds to the inputType
        google::protobuf::DynamicMessageFactory dynamicFactory;

        // Prepare the RPC call:
        std::multimap<grpc::string, grpc::string> clientMetadata;
        std::string methodStr = "/" + serviceName + "/" + methodName;
        grpc::testing::CliCall call(channel, methodStr, clientMetadata);
        auto messageFormatter = createOutputFormatter(parseTree);
        auto messageParser = createMessageParser(parseTree);

        // Parse request messages given by the user:
        auto requestMessages = messageParser->parseMessages(parseTree, dynamicFactory, inputType, method->client_streaming());
        if(requestMessages.size() == 0 and not method->client_streaming())
        {
            std::cerr << "Error parsing method arguments -> aborting the call :-(" << std::endl;
            return -1;
        }

        if(not method->client_streaming() and requestMessages.size() > 1)
        {
            std::cerr << "Error: For Unary calls only one request message is allowed. -> aborting the call :-(" << std::endl;
            return -1;
        }

        // Write all request messages (multiple in case of request stream)
        for (auto & message : requestMessages)
        {
            if (parseTree.findFirstChild("PrintParsedMessage") != "")
            {
                std::cout << "Request message:" << std::endl
                          << messageFormatter->messageToString(*message, method->input_type()) << std::endl;
            }

            // now we serialize the message:
            grpc::string serializedRequest;
            bool success = message->SerializeToString(&serializedRequest);
            if (not success)
            {
                std::cerr << "Error: Failed to serialize method arguments" << std::endl;
                return -1;
            }

            call.Write(serializedRequest);
        }

        // End the request stream. (This is a limitation of gWhisper streaming support, as we sequentially stream all request messages, then end the stream and then handle the reply stream.) No async streaming is possible via this CLI at the moment.
        call.WritesDone();

        // In a loop we read reply data from the reply stream:
        // NOTE: in gRPC every RPC can be considered "streaming". Non-streaming RPCs
        //  merely return one reply message.
        grpc::string serializedResponse;
        std::multimap<grpc::string_ref, grpc::string_ref> serverMetadataA;
        bool init = true;
        for (init = true; call.Read(&serializedResponse, init ? &serverMetadataA : nullptr); init = false)
        {
            // convert data received from stream into a message:
            std::unique_ptr<grpc::protobuf::Message> replyMessage(dynamicFactory.GetPrototype(method->output_type())->New());
            replyMessage->ParseFromString(serializedResponse);

            // print date/time of message reception:
            std::cerr << getTimeString();
            std::cerr << ": Received message:\n";

            // print out string representation of the message:
            std::cout << messageFormatter->messageToString(*replyMessage, method->output_type());

            // Print newline after the message.
            bool customOutputFormatRequested = false;
            ArgParse::ParsedElement customFormatParseTree = parseTree.findFirstSubTree("CustomOutputFormat", customOutputFormatRequested);
            if(customOutputFormatRequested)
            {
                // in case of custom message format to cerr (to not add additional chars in case of raw/binary output):
                std::cerr << std::endl;
            }
            else
            {
                // in case of normal json or huma readable output to stdout
                std::cout << std::endl;
            }
        }

        // reply stream finished -> finish the RPC:
        std::multimap<grpc::string_ref, grpc::string_ref> serverMetadataB;
        grpc::Status status = call.Finish(&serverMetadataB);

        if (not status.ok())
        {
            std::cerr << "RPC failed ;( Status code: " << std::to_string(status.error_code()) << " " << cli::getGrpcStatusCodeAsString(status.error_code()) << ", error message: " << status.error_message() << std::endl;
            return -1;
        }

        std::cerr << "RPC succeeded :D" << std::endl;

        return 0;
    }

}

