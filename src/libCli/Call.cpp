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
#include <third_party/gRPC_utils/cli_call.h>
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

#include <libCli/cliUtils.hpp>

using namespace ArgParse;

static cli::OutputFormatter::CustomStringModifier getModifier(ArgParse::ParsedElement &f_optionalModifier);

namespace cli
{

// TODO: move this to OutputFormatting code
std::string customMessageFormat(const grpc::protobuf::Message & f_message, const grpc::protobuf::Descriptor* f_messageDescriptor, ParsedElement & f_customFormatParseTree, size_t startChild = 0)
{
    std::string result;
    const google::protobuf::Reflection * reflection = f_message.GetReflection();

    // first look for target fields to format:
    bool found = false;
    ParsedElement targetList = f_customFormatParseTree.findFirstSubTree("TargetSpecifier", found);

    if((targetList.getChildren().size() > startChild) && (targetList.getChildren()[startChild] != nullptr))
    {
        std::string partialTarget = targetList.getChildren()[startChild]->findFirstChild("PartialTarget");
        //std::cout << "looking at '" << partialTarget << "'\n";
        if(partialTarget != "")
        {
        // empty target addresses the current message
        const google::protobuf::FieldDescriptor * partialField = f_messageDescriptor->FindFieldByName(partialTarget);
        if(partialField == nullptr)
        {
            return "No such field: " + partialTarget;
        }

        // now we have three possibilities:
        // 1. repeated field
        //  -> iterate over all instances + call recursive + return
        // 2. message type field
        //  -> call recursive + return
        // 3. normal field (terminal)
        //  -> continue looping

        if(partialField->is_repeated())
        {
            //std::cout << "have repeated\n";
            switch(partialField->type())
            {
                case grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
                    {

                    }
                    break;
                default:
                    return "repeated-" + std::string(partialField->type_name()) + " is not yet supported :(\n";
                    break;
            }
            int numberOfRepetitions = reflection->FieldSize(f_message, partialField);
            for(int j = 0; j < numberOfRepetitions; j++)
            {
                //std::cout << " have repeated entry\n";
                const google::protobuf::Message & subMessage = reflection->GetRepeatedMessage(f_message, partialField, j);
                result += customMessageFormat(subMessage, partialField->message_type(), f_customFormatParseTree, startChild+1);
            }
            return result;
        }
        if(partialField->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
        {
            //std::cout << "have message\n";
            const google::protobuf::Message & subMessage = reflection->GetMessage(f_message, partialField);
            return customMessageFormat(subMessage, partialField->message_type(), f_customFormatParseTree, startChild+1);
        }
        }
    }

    // now we know we are not repeated and now f_message contains the correct
    // context in which to evaluate field references :)

    //std::cout << "have field\n";
    OutputFormatter myOutputFormatter;
    myOutputFormatter.clearColorMap();

    bool haveFormatString = false;
    auto formatString = f_customFormatParseTree.findFirstSubTree("OutputFormatString", haveFormatString);
    if(not haveFormatString)
    {
        return "Error: no format string given\n";
    }
    for(auto outputStatement : formatString.getChildren())
    {
        bool foundFieldReference = false;
        auto fieldReference = outputStatement->findFirstSubTree("OutputFieldReference", foundFieldReference);
        if(foundFieldReference)
        {
            OutputFormatter::CustomStringModifier modifier = getModifier(*outputStatement);

            //std::cout << "  have field ref " <<  fieldReference.getMatchedString() << "\n";
            // need to lookup the field:
            const google::protobuf::FieldDescriptor * fieldRef = f_messageDescriptor->FindFieldByName(fieldReference.getMatchedString());
            if(fieldRef == nullptr)
            {
                result += "???";
            }
            else
            {
                result += myOutputFormatter.fieldValueToString(f_message, fieldRef, "", "", modifier);
            }
        }
        else
        {
            //std::cout << "  have string " <<  outputStatement->getMatchedString() << "\n";
            result += outputStatement->getMatchedString();
        }
    }

    return result;
}

std::string getTimeString()
{
    // unfortunately std::chrono::system_clock::to_time_t() is not available
    // with gcc4.8. So we use std::time and std::strftime instead.
    std::time_t t = std::time(0) ;
    char cstr[128] ;
    std::strftime( cstr, sizeof(cstr), "%Y-%m-%d %X", std::localtime(&t) ) ;
    return cstr ;
}

int call(ParsedElement & parseTree)
{
    std::string serviceName = parseTree.findFirstChild("Service");
    std::string methodName = parseTree.findFirstChild("Method");
    bool argsExist;
    ParsedElement & methodArgs = parseTree.findFirstSubTree("MethodArgs", argsExist);
    std::string serverAddress = cli::getServerUri(&parseTree);

    std::shared_ptr<grpc::Channel> channel = ConnectionManager::getInstance().getChannel(serverAddress);

    if(not waitForChannelConnected(channel, getConnectTimeoutMs(&parseTree)))
    {
        std::cerr << "Error: channel connection attempt timed out" << std::endl;
        return -1;
    }

    const grpc::protobuf::ServiceDescriptor* service = ConnectionManager::getInstance().getDescPool(serverAddress)->FindServiceByName(serviceName);
    if(service == nullptr)
    {
        std::cerr << "Error: Service '" << serviceName << "' not found" << std::endl;
        return -1;
    }

    auto method = service->FindMethodByName(methodName);
    if(method == nullptr)
    {
        std::cerr << "Error: Method not found" << std::endl;
        return -1;
    }

    const grpc::protobuf::Descriptor* inputType = method->input_type();

    // now we have to construct a protobuf from the parsed argument, which corresponds to the inputType
    google::protobuf::DynamicMessageFactory dynamicFactory;

    std::vector<ArgParse::ParsedElement*> requestMessages;
    // search all passed messages: (true flag prevents searching sub-messages)
    parseTree.findAllSubTrees("Message", requestMessages, true);

    if(not method->client_streaming() and requestMessages.size() == 0)
    {
        // User did not give any message arguments for non-streaming RPC
        // In this case we just add the parseTree, which causes a default message to be cunstructed:
        requestMessages.push_back(&parseTree);
    }

    // Prepare the RPC call:
    std::multimap<grpc::string, grpc::string> clientMetadata;
    grpc::string serializedResponse;
    std::multimap<grpc::string_ref, grpc::string_ref> serverMetadataA;
    std::multimap<grpc::string_ref, grpc::string_ref> serverMetadataB;

    std::string methodStr =  "/" + serviceName + "/" + methodName;
    grpc::testing::CliCall call(channel, methodStr, clientMetadata);

    // Write all request messages (multiple in case of request stream)
    for(ArgParse::ParsedElement * messageParseTree : requestMessages)
    {
        // read data from the parse tree into the protobuf message:
        std::unique_ptr<grpc::protobuf::Message> message = cli::parseMessage(*messageParseTree, dynamicFactory, inputType);

        if(parseTree.findFirstChild("PrintParsedMessage") != "")
        {
            // use built-in human readable output format
            cli::OutputFormatter imessageFormatter;
            std::cout << "Request message:" << std::endl <<  imessageFormatter.messageToString(*message, method->input_type(), "| ", "| " ) << std::endl;
        }

        if(not message)
        {
            std::cerr << "Error: Error parsing method arguments -> aborting the call :-(" << std::endl;
            return -1;
        }

        // now we serialize the message:
        grpc::string serializedRequest;
        bool success = message->SerializeToString(&serializedRequest);
        if(not success)
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
    bool init = true;
    for (init = true; call.Read(&serializedResponse, init ? &serverMetadataA : nullptr); init= false)
    {
        // convert data received from stream into a message:
        std::unique_ptr<grpc::protobuf::Message> replyMessage(dynamicFactory.GetPrototype(method->output_type())->New());
        replyMessage->ParseFromString(serializedResponse);

        // print date/time of message reception:
        std::cerr << getTimeString();
        std::cerr << ": Received message:\n";

        // print out string representation of the message:
        std::string msgString;
        // decide on message formatting method to use:
        bool customOutputFormatRequested = false;
        ParsedElement customFormatParseTree = parseTree.findFirstSubTree("CustomOutputFormat", customOutputFormatRequested);
        if(not customOutputFormatRequested)
        {
            // use built-in human readable output format
            cli::OutputFormatter messageFormatter;

            // disable colored output if explicitly specified:
            if(parseTree.findFirstChild("NoColor") != "")
            {
                messageFormatter.clearColorMap();
            }

            // disable map output as key => value if explicitly specified:
            if(parseTree.findFirstChild("NoSimpleMapOutput") != "")
            {
                messageFormatter.disableSimpleMapOutput();
            }

            // automatically disable colored output, when outputting to something
            // else than a terminal (pipes, files, etc.), except we explicitly
            // request color mode:
            if((not isatty(fileno(stdout))) and (parseTree.findFirstChild("Color") == ""))
            {
                messageFormatter.clearColorMap();
            }

            msgString = messageFormatter.messageToString(*replyMessage, method->output_type(), "| ", "| " );
            std::cout << msgString << std::endl;
        }
        else
        {
            //std::cout << customFormatParseTree.getDebugString();
            // use user provided output format string
            msgString = customMessageFormat(*replyMessage, method->output_type(), customFormatParseTree);
            std::cout << msgString; // Omit endline here. This is an unwanted char when binary data is directed into a file.
            std::cerr << std::endl; // ... but put and endline into stderr to keep the console output nice again.
        }
    }

    // reply stream finished -> finish the RPC:
    grpc::Status status = call.Finish(&serverMetadataB);

    if(not status.ok())
    {
        std::cerr << "RPC failed ;( Status code: " << std::to_string(status.error_code()) << " " << cli::getGrpcStatusCodeAsString(status.error_code())  << ", error message: " << status.error_message() << std::endl;
        return -1;
    }

    std::cerr << "RPC succeeded :D" << std::endl;


    return 0;
}

}

/// getModifier()
///
/// @param f_optionalModifier A single child of "OutputFormatString"
/// @return                   Returns the appropriate modifier or 'Default' if non-existent
static cli::OutputFormatter::CustomStringModifier getModifier(ArgParse::ParsedElement &f_optionalModifier)
{
    cli::OutputFormatter::CustomStringModifier modifier = cli::OutputFormatter::CustomStringModifier::Default;
    bool foundModifier = false;

    auto modifierNode = f_optionalModifier.findFirstSubTree("ModifierType", foundModifier);
    if(foundModifier)
    {
        if(modifierNode.getMatchedString() == "raw")
        {
            modifier = cli::OutputFormatter::CustomStringModifier::Raw;
        }
        else if(modifierNode.getMatchedString() == "dec")
        {
            modifier = cli::OutputFormatter::CustomStringModifier::Dec;
        }
        else if(modifierNode.getMatchedString() == "default")
        {
            modifier = cli::OutputFormatter::CustomStringModifier::Default;
        }
        else if(modifierNode.getMatchedString() == "hex")
        {
            modifier = cli::OutputFormatter::CustomStringModifier::Hex;
        }
    }

    return modifier;
}
