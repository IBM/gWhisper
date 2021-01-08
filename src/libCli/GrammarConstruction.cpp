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

#include <libCli/GrammarConstruction.hpp>
#include <third_party/gRPC_utils/proto_reflection_descriptor_database.h>
#include <libCli/cliUtils.hpp>
#include <libCli/ConnectionManager.hpp>
#include "protoDoc/protoDoc.pb.h"

using namespace ArgParse;

namespace cli
{

std::string getServerUri(ParsedElement * f_parseTree)
{
    std::string serverUri = f_parseTree->findFirstChild("ServerUri");
    if(f_parseTree->findFirstChild("TcpUri") != "" and f_parseTree->findFirstChild("TcpPort") == "")
    {
        serverUri += ":50051";
    }
    //serverUri = "127.0.0.1:50051";
    return serverUri;
}

class GrammarInjectorMethodArgs : public GrammarInjector
{
    public:
        GrammarInjectorMethodArgs(Grammar & f_grammar, const std::string & f_elementName = "") :
            GrammarInjector("MethodArgs", f_elementName),
            m_grammar(f_grammar)
        {
        }

        virtual ~GrammarInjectorMethodArgs()
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {
            // FIXME: we are already completing this without a service parsed.
            //  this works in most cases, as it will just fail. however this is not really a nice thing.
            std::string serviceName = f_parseTree->findFirstChild("Service");
            std::string methodName = f_parseTree->findFirstChild("Method");
            std::string serverAddress = getServerUri(f_parseTree);

            //std::cout << f_parseTree->getDebugString() << std::endl;
            //std::cout << "Injecting grammar for " << serverAddress << ":" << serverPort << " " << serviceName << " " << methodName << std::endl;
            std::shared_ptr<grpc::Channel> channel = ConnectionManager::getInstance().getChannel(serverAddress);

            if(not waitForChannelConnected(channel, getConnectTimeoutMs(f_parseTree)))
            {
                f_ErrorMessage = "Error: Could not connect the Server.";
                return nullptr;
            }

            const grpc::protobuf::ServiceDescriptor* service = ConnectionManager::getInstance().getDescPool(serverAddress)->FindServiceByName(serviceName);

            if(service == nullptr)
            {
                f_ErrorMessage = "Error: Service not found.";
                return nullptr;
            }

            auto method = service->FindMethodByName(methodName);
            if(method == nullptr)
            {
                f_ErrorMessage = "Error: Method not found.";
                return nullptr;
            }

            if(method->client_streaming())
            {
                ArgParse::GrammarFactory grammarFactory(m_grammar);

                return grammarFactory.createList(
                    "RequestStream",
                    getMessageGrammar("Message", method->input_type(), m_grammar.createElement<FixedString>(":")),
                    m_grammar.createElement<WhiteSpace>(),
                    false,
                    nullptr,
                    nullptr
                    );
            }
            else
            {
                return getMessageGrammar("Message", method->input_type());
            }
            //auto concat = m_grammar.createElement<Concatenation>();

            //auto separation = m_grammar.createElement<WhiteSpace>();
            ////auto separation = m_grammar.createElement<Alternation>();
            ////separation->addChild(m_grammar.createElement<WhiteSpace>());
            ////separation->addChild(m_grammar.createElement<FixedString>(","));
            //concat->addChild(separation);

            //concat->addChild(fields);

            //auto result = m_grammar.createElement<Repetition>("Fields");
            //result->addChild(concat);

            //return result;
        };

    private:

        void addFieldValueGrammar(GrammarElement * f_fieldGrammar, const grpc::protobuf::FieldDescriptor * f_field)
        {

            switch(f_field->cpp_type())
            {
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_FLOAT:
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("[\\+-\\.pP0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_DOUBLE:
                    // TODO: make regex match closer
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("[\\+-\\.pP0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("[\\+-]?(0x|0X|0b)?[0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_INT64:
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("[\\+-]?(0x|0X|0b)?[0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_UINT32:
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("\\+?(0x|0X|0b)?[0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_UINT64:
                    f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("\\+?(0x|0X|0b)?[0-9a-fA-F]+", "FieldValue"));
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_BOOL:
                    {
                        auto boolGrammar = m_grammar.createElement<Alternation>("FieldValue");
                        boolGrammar->addChild(m_grammar.createElement<FixedString>("true"));
                        boolGrammar->addChild(m_grammar.createElement<FixedString>("false"));
                        boolGrammar->addChild(m_grammar.createElement<FixedString>("1"));
                        boolGrammar->addChild(m_grammar.createElement<FixedString>("0"));
                        f_fieldGrammar->addChild(boolGrammar);
                        break;
                    }
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_ENUM:
                    {
                        const google::protobuf::EnumDescriptor * enumDesc = f_field->enum_type();
                        auto enumGrammar = m_grammar.createElement<Alternation>("FieldValue");
                        for(int i = 0; i<enumDesc->value_count(); i++)
                        {
                            const google::protobuf::EnumValueDescriptor * enumValueDesc = enumDesc->value(i);
                            // FIXME: null possible?
                            enumGrammar->addChild(m_grammar.createElement<FixedString>(enumValueDesc->name()));
                        }
                        f_fieldGrammar->addChild(enumGrammar);
                        break;
                    }
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:
                    if(f_field->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_BYTES)
                    {
                        auto bytesContainer = m_grammar.createElement<Alternation>("FieldValue");

                        auto bytesContainerHexString = m_grammar.createElement<Concatenation>("FieldValueHexString");
                        bytesContainerHexString->addChild(m_grammar.createElement<FixedString>("0x"));
                        bytesContainerHexString->addChild(m_grammar.createElement<RegEx>("[0-9a-fA-F]*", ""));

                        auto bytesContainerFileInput = m_grammar.createElement<Concatenation>("FieldValueFileInput");
                        bytesContainerFileInput->addChild(m_grammar.createElement<FixedString>("file://"));
                        bytesContainerFileInput->addChild(m_grammar.createElement<RegEx>("[^:, ]*", ""));

                        bytesContainer->addChild(bytesContainerHexString);
                        bytesContainer->addChild(bytesContainerFileInput);

                        f_fieldGrammar->addChild(bytesContainer);
                    }
                    else
                    {
                        // FIXME: commented solution does not work:
                        //        we always complete "::" as empty string matches
                        //        Solution would be to change the parser to never
                        //        attempt completion when a regex is currently
                        //        parsed. This requires changes in the parser
                        //        and could not be implemented on short notice.
                        //auto stringContainer = m_grammar.createElement<Concatenation>("StringContainer");
                        //stringContainer->addChild(m_grammar.createElement<FixedString>(":"));
                        //stringContainer->addChild(m_grammar.createElement<RegEx>("[^:]*", "FieldValue"));
                        //stringContainer->addChild(m_grammar.createElement<FixedString>(":"));
                        //f_fieldGrammar->addChild(stringContainer);

                        // Using this as a workaround until parser gets better regex support
                        //f_fieldGrammar->addChild(m_grammar.createElement<RegEx>("[^:, ]*", "FieldValue"));
                        f_fieldGrammar->addChild(m_grammar.createElement<EscapedString>(":, ", ':', "FieldValue"));
                    }
                    break;
                case grpc::protobuf::FieldDescriptor::CppType::CPPTYPE_MESSAGE:
                    {
                        ArgParse::GrammarFactory grammarFactory(m_grammar);

                        //auto fieldsAlt = getMessageGrammar(f_field->message_type());
                        auto subMessage = getMessageGrammar("FieldValue", f_field->message_type(), m_grammar.createElement<FixedString>(":"));
                        //auto  prepostfix = m_grammar.createElement<FixedString>(":");
                        //GrammarElement * subMessage = grammarFactory.createList(
                        //        "FieldValue",
                        //        fieldsAlt,
                        //        m_grammar.createElement<WhiteSpace>(),
                        //        false,
                        //        prepostfix,
                        //        prepostfix
                        //        );

                        ////std::cerr << "Field '" << field->name() << "' has message type: '" << field->type_name() << "'" << std::endl;
                        //auto subMessage = m_grammar.createElement<Concatenation>("FieldValue");
                        //subMessage->addChild(m_grammar.createElement<FixedString>(":"));

                        //auto childFieldsRep = m_grammar.createElement<Repetition>("Fields");
                        //auto concat = m_grammar.createElement<Concatenation>();
                        //auto fieldsAlt = getMessageGrammar(f_field->message_type());
                        //concat->addChild(fieldsAlt);

                        //auto separation = m_grammar.createElement<WhiteSpace>();
                        ////auto separation = m_grammar.createElement<Alternation>();
                        ////separation->addChild(m_grammar.createElement<WhiteSpace>());
                        ////separation->addChild(m_grammar.createElement<FixedString>(","));
                        //concat->addChild(separation);

                        //childFieldsRep->addChild(concat);
                        //subMessage->addChild(childFieldsRep);

                        //subMessage->addChild(m_grammar.createElement<FixedString>(":"));
                        f_fieldGrammar->addChild(subMessage);
                        break;
                    }
                default:
                    std::cerr << "Field '" << f_field->name() << "' has unsupported type: '" << f_field->type_name() << "'" << std::endl;
                    break;
            }

            if(f_field->options().GetExtension(field_doc).empty())
            {
                // add default documentation, if no explicit docstring is given
                f_fieldGrammar->setDocument(f_field->type_name());
            }
        }

        // FIXME: we do want to generate a list via factory here not an alternation.
        // This makes life much easier and avoids duplicate code as all messages have
        // same parse structure.
        GrammarElement * getMessageGrammar(const std::string & f_rootElementName, const grpc::protobuf::Descriptor* f_messageDescriptor, GrammarElement * f_wrappingElement = nullptr)
        {
            ArgParse::GrammarFactory grammarFactory(m_grammar);
            auto fieldsAlt = m_grammar.createElement<Alternation>();
            //auto  prepostfix = m_grammar.createElement<FixedString>(":");
            GrammarElement * message = grammarFactory.createList(
                    f_rootElementName,
                    fieldsAlt,
                    m_grammar.createElement<WhiteSpace>(),
                    false,
                    f_wrappingElement,
                    f_wrappingElement
                    );

            // iterate over fields:
            for(int i = 0; i< f_messageDescriptor->field_count(); i++)
            {
                const grpc::protobuf::FieldDescriptor * field = f_messageDescriptor->field(i);

                //std::cerr << "Iterating field " << std::to_string(i) << " of message " << f_messageDescriptor->name() << "with name: '" << field->name() <<"'"<< std::endl;

                // now we add grammar to the fieldsAlt alternation:
                auto fieldGrammar = m_grammar.createElement<Concatenation>("Field");
                fieldGrammar->addChild(m_grammar.createElement<FixedString>(field->name(), "FieldName"));
                fieldGrammar->addChild(m_grammar.createElement<FixedString>("="));
                fieldsAlt->addChild(fieldGrammar);
                fieldGrammar->setDocument(field->options().GetExtension(field_doc));//get the in the custom filed option of .proto definited document and set it into the grammer.
                if(field->is_repeated())
                {
                    auto repeatedValue = m_grammar.createElement<Concatenation>("RepeatedValue");
                    addFieldValueGrammar(repeatedValue, field);

                    auto repeatedGrammar = m_grammar.createElement<Concatenation>("FieldValue");
                    repeatedGrammar->addChild(m_grammar.createElement<FixedString>(":"));

                    repeatedGrammar->addChild(repeatedValue);

                    auto repeatedOptionalEntry = m_grammar.createElement<Concatenation>();
                    repeatedOptionalEntry->addChild(m_grammar.createElement<FixedString>(","));
                    repeatedOptionalEntry->addChild(m_grammar.createElement<WhiteSpace>());
                    repeatedOptionalEntry->addChild(repeatedValue);

                    auto repeatedOptionalValues = m_grammar.createElement<Repetition>();
                    repeatedOptionalValues->addChild(repeatedOptionalEntry);
                    repeatedGrammar->addChild(repeatedOptionalValues);
                    repeatedGrammar->addChild(m_grammar.createElement<FixedString>(":"));


                    fieldGrammar->addChild(repeatedGrammar);
                }
                else
                {
                    // the simple case:
                    addFieldValueGrammar(fieldGrammar, field);
                }
            }

            //std::cout << "Grammar generated:\n" << fieldsAlt->toString() << std::endl;
            return message;
        }



        Grammar & m_grammar;

};

class GrammarInjectorMethods : public GrammarInjector
{
    public:
        GrammarInjectorMethods(Grammar & f_grammar, const std::string & f_elementName = "") :
            GrammarInjector("Method", f_elementName),
            m_grammar(f_grammar)
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {
            // FIXME: we are already completing this without a service parsed.
            //  this works in most cases, as it will just fail. however this is not really a nice thing.
            std::string serviceName = f_parseTree->findFirstChild("Service");
            std::string serverAddress = getServerUri(f_parseTree);
            //std::cout << f_parseTree->getDebugString() << std::endl;
            //std::cout << "Injecting grammar for " << serverAddress << ":" << serverPort << " " << serviceName << std::endl;
            std::shared_ptr<grpc::Channel> channel = ConnectionManager::getInstance().getChannel(serverAddress);

            if(not waitForChannelConnected(channel, getConnectTimeoutMs(f_parseTree)))
            {
                f_ErrorMessage = "Error: Could not connect the Server.";
                return nullptr;
            }

            const grpc::protobuf::ServiceDescriptor* service = ConnectionManager::getInstance().getDescPool(serverAddress)->FindServiceByName(serviceName);
            auto result = m_grammar.createElement<Alternation>();
            if(service != nullptr)
            {
                for (int i = 0; i < service->method_count(); ++i)
                {
                    auto childAlt = m_grammar.createElement<FixedString>(service->method(i)->name());
                    //childAlt->setDocument(service->method(i)->input_type()->options().GetExtension(rpc_doc))//custom option in protoDoc: message_doc
                    childAlt->setDocument(service->method(i)->options().GetExtension(rpc_doc));//grpc field_doc (methodcustom option in protoDoc: method_doc)
                    result->addChild(childAlt);
                }
            }
            else
            {
                f_ErrorMessage = "Error: Service not found.";
                return nullptr;
            }
            return result;
        };

    private:
        Grammar & m_grammar;

};

class GrammarInjectorServices : public GrammarInjector
{
    public:
        GrammarInjectorServices(Grammar & f_grammar, const std::string & f_elementName = "") :
            GrammarInjector("Service", f_elementName),
            m_grammar(f_grammar)
        {
        }

        virtual ~GrammarInjectorServices()
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {
            std::string serverAddress = getServerUri(f_parseTree);

            //std::cout << "Injecting Service grammar for " << serverAddress << std::endl;
            std::shared_ptr<grpc::Channel> channel = ConnectionManager::getInstance().getChannel(serverAddress);

            if(not waitForChannelConnected(channel, getConnectTimeoutMs(f_parseTree)))
            {
                f_ErrorMessage = "Error: Server not found.";
                return nullptr;
            }

            std::vector<grpc::string> serviceList;
            if(not ConnectionManager::getInstance().getDescDb(serverAddress)->GetServices(&serviceList))
            {
                f_ErrorMessage = "Error: Could not retrieve service list.";
                return nullptr;
            }

            auto result = m_grammar.createElement<Alternation>();
            for(auto service : serviceList)
            {
                auto childAlt = m_grammar.createElement<FixedString>(service);
                const grpc::protobuf::ServiceDescriptor* m_service = ConnectionManager::getInstance().getDescPool(serverAddress)->FindServiceByName(service);
                childAlt->setDocument(m_service->options().GetExtension(service_doc));
                result->addChild(childAlt);
            }
            //std::cout << "result = " << result <<std::endl;
            return result;
        };

    private:
        Grammar & m_grammar;

};

GrammarElement * constructGrammar(Grammar & f_grammarPool)
{
    // user defined output formatting
    // something like this will match: @.fru_info_list:found fru in slot /slot_id/:
    GrammarElement * formatTargetSpecifier = f_grammarPool.createElement<Concatenation>("CustomOutputFormat");
    formatTargetSpecifier->addChild(f_grammarPool.createElement<FixedString>("@"));
    GrammarElement * formatTargetTree = f_grammarPool.createElement<Repetition>("TargetSpecifier");
    GrammarElement * formatTargetPart = f_grammarPool.createElement<Concatenation>();
    formatTargetPart->addChild(f_grammarPool.createElement<FixedString>("."));
    formatTargetPart->addChild(f_grammarPool.createElement<RegEx>("[^:]*", "PartialTarget"));
    formatTargetTree->addChild(formatTargetPart);
    formatTargetSpecifier->addChild(formatTargetTree);
    formatTargetSpecifier->addChild(f_grammarPool.createElement<FixedString>(":"));
    GrammarElement * formatOutputSpecifier = f_grammarPool.createElement<Repetition>("OutputFormatString");
    GrammarElement * formatOutputSpecifierAlternation = f_grammarPool.createElement<Alternation>();
    formatOutputSpecifierAlternation->addChild(f_grammarPool.createElement<RegEx>("[^:/]+", "OutputFixedString")); // a real string
    GrammarElement * fieldReference = f_grammarPool.createElement<Concatenation>();
    fieldReference->addChild(f_grammarPool.createElement<FixedString>("/"));
    fieldReference->addChild(f_grammarPool.createElement<RegEx>("[^/,%=]+", "OutputFieldReference")); // field reference


    // Modifier = { % <ModifierType> [, <ModifierPaddingAlt> = [0-9]+ ] }
    GrammarElement * modifier = f_grammarPool.createElement<Optional>("Modifier");                   // contains
    GrammarElement * modifierConcat = f_grammarPool.createElement<Concatenation>("ModifierConcat");  // contains
    GrammarElement * modifierType = f_grammarPool.createElement<Alternation>("ModifierType");

    modifierType->addChild(f_grammarPool.createElement<FixedString>("default"));
    modifierType->addChild(f_grammarPool.createElement<FixedString>("raw"));
    modifierType->addChild(f_grammarPool.createElement<FixedString>("dec"));
    modifierType->addChild(f_grammarPool.createElement<FixedString>("hex"));

    modifierConcat->addChild(f_grammarPool.createElement<FixedString>("%"));
    modifierConcat->addChild(modifierType);

//    GrammarElement * modifierPaddingOpt = f_grammarPool.createElement<Optional>("ModifierPadding");
//    GrammarElement * modifierPaddingConcat = f_grammarPool.createElement<Concatenation>("ModifierPaddingConcat");
//    GrammarElement * modifierPaddingAlt = f_grammarPool.createElement<Alternation>("ModifierPaddingType");
//    modifierPaddingAlt->addChild(f_grammarPool.createElement<FixedString>("zeroPaddingChars"));
//    modifierPaddingAlt->addChild(f_grammarPool.createElement<FixedString>("spacePaddingChars"));
//
//    modifierPaddingConcat->addChild(f_grammarPool.createElement<FixedString>(","));
//    modifierPaddingConcat->addChild(modifierPaddingAlt);
//    modifierPaddingConcat->addChild(f_grammarPool.createElement<FixedString>("="));
//    modifierPaddingConcat->addChild(f_grammarPool.createElement<RegEx>("[0-9]+", "NumPaddingCharacters"));
//
//    modifierPaddingOpt->addChild(modifierPaddingConcat);
//    modifierConcat->addChild(modifierPaddingOpt);

    modifier->addChild(modifierConcat);
    fieldReference->addChild(modifier);
    fieldReference->addChild(f_grammarPool.createElement<FixedString>("/"));
    formatOutputSpecifierAlternation->addChild(fieldReference);
    formatOutputSpecifier->addChild(formatOutputSpecifierAlternation);

    formatTargetSpecifier->addChild(formatOutputSpecifier);
    formatTargetSpecifier->addChild(f_grammarPool.createElement<FixedString>(":"));

    GrammarElement * customOutputFormat = f_grammarPool.createElement<Concatenation>();
    customOutputFormat->addChild(f_grammarPool.createElement<FixedString>("--customOutput"));
    customOutputFormat->addChild(f_grammarPool.createElement<WhiteSpace>());
    customOutputFormat->addChild(formatTargetSpecifier);
    // TODO add this to options

    // options
    GrammarElement * options = f_grammarPool.createElement<Repetition>(); // TODO: support multiple options
    GrammarElement * optionsconcat = f_grammarPool.createElement<Concatenation>();
    GrammarElement * optionsalt = f_grammarPool.createElement<Alternation>();
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("-h", "Help"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--help", "Help"));

    GrammarElement * completeOption = f_grammarPool.createElement<Concatenation>();
    completeOption->addChild(f_grammarPool.createElement<FixedString>("--complete", "Complete"));
    GrammarElement * completeDialectOption = f_grammarPool.createElement<Optional>();
    completeOption->addChild(completeDialectOption);
    GrammarElement * completeDialectConcat = f_grammarPool.createElement<Concatenation>();
    completeDialectOption->addChild(completeDialectConcat);
    completeDialectConcat->addChild(f_grammarPool.createElement<FixedString>("="));
    GrammarElement * completeDialectChoice = f_grammarPool.createElement<Alternation>("CompleteDialect");
    completeDialectConcat->addChild(completeDialectChoice);
    completeDialectChoice->addChild(f_grammarPool.createElement<FixedString>("bash", "bash"));
    completeDialectChoice->addChild(f_grammarPool.createElement<FixedString>("fish", "fish"));
    optionsalt->addChild(completeOption);


    //completeOption->addChild(f_grammarPool.createElement<FixedString>("--complete", "Complete"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--debugComplete", "CompleteDebug"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--dot", "DotExport"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--noColor", "NoColor"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--color", "Color"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--version", "Version"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--printParsedMessage", "PrintParsedMessage"));
    optionsalt->addChild(f_grammarPool.createElement<FixedString>("--noSimpleMapOutput", "NoSimpleMapOutput"));
    GrammarElement * timeoutOption = f_grammarPool.createElement<Concatenation>();
    timeoutOption->addChild(f_grammarPool.createElement<FixedString>("--connectTimeoutMilliseconds="));
    timeoutOption->addChild(f_grammarPool.createElement<RegEx>("[0-9]+", "connectTimeout"));
    optionsalt->addChild(timeoutOption);
    optionsalt->addChild(customOutputFormat);
    // FIXME FIXME FIXME: we cannot distinguish between --complete and --completeDebug.. this is a problem for arguments too, as we cannot guarantee, that we do not have an argument starting with the name of an other argument.
    // -> could solve by makeing FixedString greedy
    optionsconcat->addChild(optionsalt);
    optionsconcat->addChild(
                f_grammarPool.createElement<WhiteSpace>()
            );
    //optionsconcat->addChild(
    //        f_grammarPool.createElement<Optional>()->addChild(
    //            f_grammarPool.createElement<WhiteSpace>()
    //            )
    //        );
    options->addChild(optionsconcat);

    // Server address
    // We parse gRPC URIs roughly according to https://grpc.github.io/grpc/cpp/md_doc_naming.html
    GrammarElement * serverUri = f_grammarPool.createElement<Alternation>("ServerUri");

    // unix URIs:
    GrammarElement * unixUri = f_grammarPool.createElement<Alternation>("UnixUri");

    GrammarElement * unixUriSimple = f_grammarPool.createElement<Concatenation>();
    unixUriSimple->addChild(f_grammarPool.createElement<FixedString>("unix:"));
    unixUriSimple->addChild(f_grammarPool.createElement<RegEx>("[^ ]+"));
    unixUri->addChild(unixUriSimple);

    GrammarElement * unixUriAbstract = f_grammarPool.createElement<Concatenation>();
    unixUriAbstract->addChild(f_grammarPool.createElement<FixedString>("unix-abstract:"));
    unixUriAbstract->addChild(f_grammarPool.createElement<RegEx>("[^ ]+"));
    unixUri->addChild(unixUriAbstract);

    // TCP URIs:
    GrammarElement * tcpUri = f_grammarPool.createElement<Alternation>("TcpUri");

    GrammarElement * dnsUri = f_grammarPool.createElement<Concatenation>();
    GrammarElement * dnsIdentifier = f_grammarPool.createElement<Optional>();
    dnsIdentifier->addChild(f_grammarPool.createElement<FixedString>("dns:"));
    dnsUri->addChild(dnsIdentifier);
    dnsUri->addChild(f_grammarPool.createElement<RegEx>("[^:\\[\\] ]+", "Hostname"));
    tcpUri->addChild(dnsUri);

    GrammarElement * ipv4Uri = f_grammarPool.createElement<Concatenation>();
    ipv4Uri->addChild(f_grammarPool.createElement<FixedString>("ipv4:"));
    ipv4Uri->addChild(f_grammarPool.createElement<RegEx>("\\d+\\.\\d+\\.\\d+\\.\\d+", "IPv4Address"));
    tcpUri->addChild(ipv4Uri);

    GrammarElement * ipv6Uri = f_grammarPool.createElement<Concatenation>();
    ipv6Uri->addChild(f_grammarPool.createElement<FixedString>("ipv6:"));
    ipv6Uri->addChild(f_grammarPool.createElement<RegEx>("\\[?[0-9a-fA-F:]+\\]?", "IPv6Address"));
    tcpUri->addChild(ipv6Uri);

    GrammarElement * cServerPort = f_grammarPool.createElement<Concatenation>();
    cServerPort->addChild(f_grammarPool.createElement<FixedString>(":"));
    cServerPort->addChild(f_grammarPool.createElement<RegEx>("\\d+", "TcpPort"));
    GrammarElement * serverPort = f_grammarPool.createElement<Optional>();
    serverPort->addChild(cServerPort);
    tcpUri->addChild(serverPort);


    // a server uri can either be a unix uri or a tcp uri.
    // tcp uris can be dns, ipv4 or ipv6 uris all with optional port
    serverUri->addChild(unixUri);
    serverUri->addChild(tcpUri);


    //GrammarElement * testAlt = f_grammarPool.createElement<Alternation>("TestAlt");
    //testAlt->addChild(f_grammarPool.createElement<FixedString>("challo"));
    //testAlt->addChild(f_grammarPool.createElement<FixedString>("ctschuess"));

    // main concat:
    GrammarElement * cmain = f_grammarPool.createElement<Concatenation>();
    cmain->addChild(options);
    cmain->addChild(serverUri);
    //cmain->addChild(testAlt);
    //cmain->addChild(f_grammarPool.createElement<RegEx>(std::regex("\\S+"), "ServerAddress"));
    cmain->addChild(f_grammarPool.createElement<WhiteSpace>());
    cmain->addChild(f_grammarPool.createElement<GrammarInjectorServices>(f_grammarPool, "Service"));
    cmain->addChild(f_grammarPool.createElement<WhiteSpace>());
    cmain->addChild(f_grammarPool.createElement<GrammarInjectorMethods>(f_grammarPool, "Method"));

    GrammarElement * optionalArgs = f_grammarPool.createElement<Optional>();

    GrammarElement * args = f_grammarPool.createElement<Concatenation>();
    args->addChild(f_grammarPool.createElement<WhiteSpace>());
    args->addChild(f_grammarPool.createElement<GrammarInjectorMethodArgs>(f_grammarPool, "MethodArgs"));

    optionalArgs->addChild(args);

    cmain->addChild(optionalArgs);

    return cmain;
}
}
