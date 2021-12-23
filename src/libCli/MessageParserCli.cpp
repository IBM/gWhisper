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

#include <libCli/MessageParser.hpp>
#include <fstream>
#include <exception>

using namespace ArgParse;

static int parseBytesFieldFromFile(std::string &f_resultString, const std::string &f_valueString, const std::string &f_fieldName);
static int parseBytesFieldFromHexStr(std::string &f_resultString, const std::string &f_valueString, const std::string &f_fieldName);

namespace cli
{

/// Parses a single field falue from a given parse tree into a protobuf message.
/// @param f_parseTree Parse tree containing the field value information.
/// @param f_message protobuf message to which the field value should be added
/// @param f_factory Factory for creation of additional messages (required for nested messages)
/// @param f_fieldDescriptor Descriptor describing the type of the field
/// @param f_isRepeated if true, field value will be added as a repeated field value.
///        (protobuf reflection api unfortunately does not provide a combined API for setting unique fields and adding to repeated fields)
/// @returns 0 if field value could be added to the message. -1 otherwise.
int MessageParserCli::parseFieldValue(
        ParsedElement & f_parseTree, google::protobuf::Message * f_message,
        google::protobuf::DynamicMessageFactory & f_factory,
        const google::protobuf::FieldDescriptor * f_fieldDescriptor,
        bool f_isRepeated
        )
{
    const google::protobuf::Reflection *reflection = f_message->GetReflection();
    std::string valueString = f_parseTree.findFirstChild("FieldValue");
    switch(f_fieldDescriptor->cpp_type())
    {
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_FLOAT:
            {
                float value;
                try
                {
                    value = std::stof(valueString);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing float for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddFloat(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetFloat(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_DOUBLE:
            {
                double value;
                try
                {
                    value = std::stod(valueString);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing float for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddDouble(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetDouble(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:
            {
                long value;
                try
                {
                    value = std::stol(valueString, 0, 0);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing integer for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddInt32(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetInt32(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT64:
            {
                long value;
                try
                {
                    value = std::stol(valueString, 0, 0);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing integer for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddInt64(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetInt64(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_UINT32:
            {
                unsigned long value;
                try
                {
                    value = std::stoul(valueString, 0, 0);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing integer for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddUInt32(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetUInt32(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_UINT64:
            {
                unsigned long value;
                try
                {
                    value = std::stoul(valueString, 0, 0);
                }
                catch(std::exception& e)
                {
                    std::cerr << "Error parsing integer for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
                if(f_isRepeated)
                {
                    reflection->AddUInt64(f_message, f_fieldDescriptor, value);
                }
                else
                {
                    reflection->SetUInt64(f_message, f_fieldDescriptor, value);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_BOOL:
            if( valueString == "1" || valueString == "true" || valueString == "True" )
            {
                if(f_isRepeated)
                {
                    reflection->AddBool(f_message, f_fieldDescriptor, true);
                }
                else
                {
                    reflection->SetBool(f_message, f_fieldDescriptor, true);
                }
            }
            else
            {
                if(f_isRepeated)
                {
                    reflection->AddBool(f_message, f_fieldDescriptor, false);
                }
                else
                {
                    reflection->SetBool(f_message, f_fieldDescriptor, false);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_ENUM:
            {
                const google::protobuf::EnumValueDescriptor * enumVal = f_fieldDescriptor->enum_type()->FindValueByName(valueString);
                if(enumVal != nullptr)
                {
                if(f_isRepeated)
                {
                    reflection->AddEnum(f_message, f_fieldDescriptor, enumVal);
                }
                else
                {
                    reflection->SetEnum(f_message, f_fieldDescriptor, enumVal);
                }
                }
                else
                {
                    std::cerr << "Error parsing enum for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:
            // we could have a string or a bytes input here
            if(f_fieldDescriptor->type() == google::protobuf::FieldDescriptor::Type::TYPE_BYTES)
            {
                std::string resultString;
                // if we have a bytes field, we parse a hex string or file input:
                if(valueString.substr(0,2) == "0x")
                {
                    int rc = parseBytesFieldFromHexStr(resultString, valueString, f_fieldDescriptor->name());
                    if(rc) return rc;
                }
                else if(valueString.substr(0,7) == "file://")
                {
                    int rc = parseBytesFieldFromFile(resultString, valueString, f_fieldDescriptor->name());
                    if(rc) return rc;
                }
                else
                {
                    std::cerr << "Error parsing bytes field '" << f_fieldDescriptor->name() << "': Given value does not start with '0x' nor with 'file://'." << std::endl;
                    return -1;
                }

                if(f_isRepeated)
                {
                    reflection->AddString(f_message, f_fieldDescriptor, resultString);
                }
                else
                {
                    reflection->SetString(f_message, f_fieldDescriptor, resultString);
                }
            }
            else
            {
                // otherwise we directly parse the given string:
                if(f_isRepeated)
                {
                    reflection->AddString(f_message, f_fieldDescriptor, valueString);
                }
                else
                {
                    reflection->SetString(f_message, f_fieldDescriptor, valueString);
                }
            }
            break;
        case google::protobuf::FieldDescriptor::CppType::CPPTYPE_MESSAGE:
            {
                const google::protobuf::Descriptor * subMessageDescriptor = f_fieldDescriptor->message_type();
                std::unique_ptr<google::protobuf::Message> subMessage = parseMessage(f_parseTree, f_factory, subMessageDescriptor);
                if(subMessage != nullptr)
                {
                    if(f_isRepeated)
                    {
                        reflection->AddAllocatedMessage(f_message, f_fieldDescriptor, subMessage.release());
                    }
                    else
                    {
                        reflection->SetAllocatedMessage(f_message, subMessage.release(), f_fieldDescriptor);
                    }
                }
                else
                {
                    std::cerr << "Error parsing sub-message for field '" << f_fieldDescriptor->name() << "'" << std::endl;
                    return -1;
                }
            }
            break;
        default:
            std::cerr << "Error: Parsing Field '" << f_fieldDescriptor->name() << "'. It has the unsupported type: '" << f_fieldDescriptor->type_name() << "'" << std::endl;
            return -1;
    }

    return 0;
}

std::unique_ptr<google::protobuf::Message> MessageParserCli::parseMessage(ParsedElement & f_parseTree, google::protobuf::DynamicMessageFactory & f_factory, const google::protobuf::Descriptor* f_messageDescriptor)
{
    std::unique_ptr<google::protobuf::Message> message(f_factory.GetPrototype(f_messageDescriptor)->New());

    // we iterate over all fields:
    //std::cout << "Parsing message from tree: \n" << f_parseTree.getDebugString(" ") << std::endl;
    int rc = 0;
    std::vector<ArgParse::ParsedElement*> fields;
    // search all fields: (do not search deeper if field is found to avoid searching sub-fields)
    f_parseTree.findAllSubTrees("Field", fields, true); 
    if((fields.size() == 0) and (f_messageDescriptor->field_count() > 0) )
    {
        std::cerr << "Warning: no Fields found in parseTree for message '" << f_messageDescriptor->name() << "'" << std::endl;
    }
    for(ParsedElement * parsedField : fields)
    {
        if(parsedField->isCompletelyParsed())
        {
            //std::cout << "Parsing field from tree: \n" << parsedField->getDebugString(" ") << std::endl;
            const google::protobuf::FieldDescriptor * fieldDescriptor = f_messageDescriptor->FindFieldByName(parsedField->findFirstChild("FieldName", 1));
            if(fieldDescriptor == nullptr)
            {
                std::cerr << "Warning: Field '" << parsedField->findFirstChild("FieldName", 1) << "' does not exist. Ignoring." << std::endl;
                continue;
            }

            // now we have to parse the field value according to its type:
            bool found = false;
            ParsedElement & fieldValue = parsedField->findFirstSubTree("FieldValue", found, 1);
            if(found)
            {
                if(fieldDescriptor->is_repeated())
                {
                    std::vector<ArgParse::ParsedElement *> repeatedFieldValues;
                    // note: the f_doNotSearchChildsOfMatchingElements flag needs to be set to true here
                    // this ensures, that we can have repeated fields as part of repeated messages
                    fieldValue.findAllSubTrees("RepeatedValue", repeatedFieldValues, true);
                    for(auto repeatedValue : repeatedFieldValues)
                    {
                        rc = parseFieldValue(*repeatedValue, message.get(), f_factory, fieldDescriptor, true);
                    }
                }
                else
                {
                    rc = parseFieldValue(fieldValue, message.get(), f_factory, fieldDescriptor);
                }
            }
            else
            {
                std::cerr << "Error: No Value given for field '" << parsedField->findFirstChild("FieldName") << "'" << std::endl;
                return nullptr;
            }
            if(rc != 0)
            {
                message.reset();
                break;
            }
        }
    }

    return std::move(message);
}

std::vector<std::unique_ptr<google::protobuf::Message>> MessageParserCli::parseMessages(
        ArgParse::ParsedElement & f_parseTree,
        google::protobuf::DynamicMessageFactory & f_factory,
        const google::protobuf::Descriptor* f_messageDescriptor,
        bool f_isClientStreamingRpc
        )
{
        std::vector<ArgParse::ParsedElement *> requestMessages;
        // search all passed messages: (true flag prevents searching sub-messages)
        f_parseTree.findAllSubTrees("Message", requestMessages, true);

        if (not f_isClientStreamingRpc and requestMessages.size() == 0)
        {
            // User did not give any message arguments for non-streaming RPC
            // In this case we just add the parseTree, which causes a default message to be cunstructed:
            requestMessages.push_back(&f_parseTree);
        }

        std::vector<std::unique_ptr<google::protobuf::Message>> result;
        // Write all request messages (multiple in case of request stream)
        for (ArgParse::ParsedElement *messageParseTree : requestMessages)
        {
            // read data from the parse tree into the protobuf message:
            std::unique_ptr<google::protobuf::Message> message = parseMessage(*messageParseTree, f_factory, f_messageDescriptor);

            if (not message)
            {
                result.clear();
                return result;
            }
            result.push_back(std::move(message));
        }
        return result;
}

}

static int parseBytesFieldFromFile(std::string &f_resultString, const std::string &f_valueString, const std::string &f_fieldName)
{
    if(f_valueString.substr(0,7) != "file://")
    {
        return -1;
    }

    std::string filename = f_valueString.substr(7, std::string::npos);
    std::ifstream instream;
    instream.open(filename.c_str(), std::ifstream::binary);

    if(!instream)
    {
        std::cout << "Error parsing bytes field '" << f_fieldName << "': Input file '" << filename << "' could not be opened." << std::endl;
        return -1;
    }

    std::stringstream outstream;
    std::copy(std::istreambuf_iterator<char>(instream),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(outstream));
    f_resultString = outstream.str();

    return 0;
}

static int parseBytesFieldFromHexStr(std::string &f_resultString, const std::string &f_valueString, const std::string &f_fieldName)
{
    int rc = 0;

    if(f_valueString.substr(0,2) != "0x")
    {
        return -1;
    }

    if(f_valueString.size()%2 != 0)
    {
        std::cerr << "Error parsing bytes field '" << f_fieldName << "': Given value is not a multiple of 8 bits long" << std::endl;
        return -1;
    }

    for(size_t pos = 2; pos+1<f_valueString.size(); pos+=2)
    {
        uint8_t byteVal = std::stoul(f_valueString.substr(pos,2), 0, 16);
        f_resultString.append(1,byteVal);
    }

    return rc;
}
