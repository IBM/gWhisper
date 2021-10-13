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

#include <libCli/OutputFormatting.hpp>

namespace cli
{

    template <typename T>
    std::string OutputFormatter::intToHexString(T f_value)
    {
        std::stringstream sstream;

        sstream << getColor(ColorClass::HexValue)
                << "0x"
                << std::hex << std::setfill ('0')
                << std::setw(2 * sizeof(T))
                << f_value
                << getColor(ColorClass::Normal);
        return sstream.str();
    }

    OutputFormatter::OutputFormatter() :
        m_isSimpleMapOutput(true),
        m_colorMap{
            {ColorClass::Normal, "\e[0m\e[39m"},
            {ColorClass::VerticalGuides, "\e[2m\e[37m"},
            {ColorClass::HorizontalGuides, "\e[2m\e[37m"},
            {ColorClass::NonRepeatedFieldName, "\e[94m"},
            {ColorClass::RepeatedFieldName, "\e[34m"},
            {ColorClass::RepeatedCount, "\e[33m"},
            {ColorClass::BoolTrue, "\e[32m"},
            {ColorClass::BoolFalse, "\e[31m"},
            {ColorClass::StringValue, "\e[33m"},
            {ColorClass::MessageTypeName, "\e[35m"},
            {ColorClass::DecimalValue, "\e[39m"},
            {ColorClass::HexValue, "\e[39m"},
            {ColorClass::EnumValue, "\e[33m"},
        }
    {

    }

    void OutputFormatter::clearColorMap()
    {
        m_colorMap.clear();
    }

    void OutputFormatter::disableSimpleMapOutput()
    {
        m_isSimpleMapOutput = false;
    }

    std::string OutputFormatter::getColor(OutputFormatter::ColorClass f_colorClass)
    {
        auto resultIt = m_colorMap.find(f_colorClass);
        if(resultIt != m_colorMap.end())
        {
            return resultIt->second;
        }
        else
        {
            return "";
        }
    }

    std::string OutputFormatter::colorize(OutputFormatter::ColorClass f_colorClass, const std::string & f_string)
    {
        return getColor(f_colorClass) + f_string + getColor(ColorClass::Normal);
    }

    std::string OutputFormatter::generateHorizontalGuide(size_t f_currentSize, size_t f_targetSize)
    {
        std::string result = getColor(ColorClass::HorizontalGuides);
        for(;f_currentSize<f_targetSize; f_currentSize++)
        {
            result += ".";
        }
        result += getColor(ColorClass::Normal);
        return result;
    }

std::string OutputFormatter::repeatedFieldValueToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, int f_fieldIndex, CustomStringModifier f_modifier)
{
    const google::protobuf::Reflection * reflection = f_message.GetReflection();
    std::string result;

    // Repeated oneof is not supported in protocoil buffers, so no need to check for it here

    switch(f_fieldDescriptor->type())
    {
        case grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
            {
                const google::protobuf::Message & subMessage = reflection->GetRepeatedMessage(f_message, f_fieldDescriptor, f_fieldIndex);
                //result += "\n" + f_currentPrefix + f_initPrefix + ":\n";
                result += colorize(ColorClass::MessageTypeName, std::string("{") + f_fieldDescriptor->message_type()->name() + "}");
                result += "\n";
                result += messageToString(subMessage,f_fieldDescriptor->message_type(), f_initPrefix, f_currentPrefix+f_initPrefix);
                //result += "\n" + f_currentPrefix + f_initPrefix + ":";

            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT32:
            {
                int32_t value = reflection->GetRepeatedInt32(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT64:
            {
                int64_t value = reflection->GetRepeatedInt64(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT32:
            {
                uint32_t value = reflection->GetRepeatedUInt32(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromUInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT64:
            {
                uint64_t value = reflection->GetRepeatedUInt64(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromUInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
            {
                float value = reflection->GetRepeatedFloat(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromFloat(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
            {
                double value = reflection->GetRepeatedDouble(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromFloat(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_BOOL:
            {
                bool value = reflection->GetRepeatedBool(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromBool(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_STRING:
            {
                std::string value = reflection->GetRepeatedString(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromString(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_ENUM:
            {
                const google::protobuf::EnumValueDescriptor * value = reflection->GetRepeatedEnum(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromEnum(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_BYTES:
            {
                std::string value = reflection->GetRepeatedString(f_message, f_fieldDescriptor, f_fieldIndex);
                result += stringFromBytes(value, f_modifier, f_currentPrefix + f_initPrefix);
            }
            break;
        default:
            return "repeated-" + std::string(f_fieldDescriptor->type_name()) + " is not yet supported :(";
            break;
    }

    return result;
}

bool OutputFormatter::isMapEntryPrimitive(const grpc::protobuf::Descriptor* f_messageDescriptor)
{
    if(f_messageDescriptor->field_count() == 2)
    {
        if(f_messageDescriptor->field(0)->name() == "key" && f_messageDescriptor->field(1)->name() == "value")
        {
            if(f_messageDescriptor->field(0)->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE || f_messageDescriptor->field(1)->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
            {
                return false;
            }
            return true;
        }
    }
    return false;
}

std::string OutputFormatter::fieldValueToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, CustomStringModifier f_modifier)
{
    const google::protobuf::Reflection * reflection = f_message.GetReflection();
    std::string result;

    // first, we need to check if this field is part of a OneOf:
    const google::protobuf::OneofDescriptor *	oneOfDesc = f_fieldDescriptor->containing_oneof();
    if(oneOfDesc != nullptr)
    {
        // yes we are part of a OneOf...
        // so, we check if we are the field which is set as the OneOfType
        if(reflection->GetOneofFieldDescriptor(f_message, oneOfDesc) != f_fieldDescriptor)
        {
            // no we are not set -> Do not continue to stringify this field,
            // as it is not set. Instead we add [NOT SET] to the field string:
            result += "[NOT SET]";
            // no need to decode any further...
            return result;
        }
    }

    switch(f_fieldDescriptor->type())
    {
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT32:
            {
                int32_t value = reflection->GetInt32(f_message, f_fieldDescriptor);
                result += stringFromInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT64:
            {
                int64_t value = reflection->GetInt64(f_message, f_fieldDescriptor);
                result += stringFromInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT32:
            {
                uint32_t value = reflection->GetUInt32(f_message, f_fieldDescriptor);
                result += stringFromUInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT64:
            {
                uint64_t value = reflection->GetUInt64(f_message, f_fieldDescriptor);
                result += stringFromUInt(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
            {
                float value = reflection->GetFloat(f_message, f_fieldDescriptor);
                result += stringFromFloat(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
            {
                double value = reflection->GetDouble(f_message, f_fieldDescriptor);
                result += stringFromFloat(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_BOOL:
            {
                bool value = reflection->GetBool(f_message, f_fieldDescriptor);
                result += stringFromBool(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_STRING:
            {
                std::string value = reflection->GetString(f_message, f_fieldDescriptor);
                result += stringFromString(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_ENUM:
            {
                const google::protobuf::EnumValueDescriptor * value = reflection->GetEnum(f_message, f_fieldDescriptor);
                result += stringFromEnum(value, f_modifier);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_BYTES:
            {
                std::string value = reflection->GetString(f_message, f_fieldDescriptor);
                result += stringFromBytes(value, f_modifier, f_currentPrefix + f_initPrefix);
            }
            break;
        case grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
            {
                // Message types are non-scalar and can be optional:
                // NOTE: this check cannot be applied to scalar types, as those
                // get optimized out if protobuf realizes they are set to default
                // value. So the `HasField` method will show false even though
                // the user set the value explicitly to the default value
                if(not reflection->HasField(f_message, f_fieldDescriptor))
                {
                    result += "[NOT SET]";
                    // no need to decode any further...
                    return result;
                }
                const google::protobuf::Message & subMessage = reflection->GetMessage(f_message, f_fieldDescriptor);
                //result += ":\n";
                result += colorize(ColorClass::MessageTypeName, std::string("{") + f_fieldDescriptor->message_type()->name() + "}");
                result += "\n";
                result += messageToString(subMessage,f_fieldDescriptor->message_type(), f_initPrefix, f_currentPrefix+f_initPrefix);
                //result += "\n" + f_currentPrefix + f_initPrefix + ":";
            }
            break;
        default:
            return std::string(f_fieldDescriptor->type_name()) + " is not yet supported :(";
            break;
    }

    return result;
}

std::string OutputFormatter::fieldToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, size_t maxFieldNameSize)
{
    std::string result;
    const google::protobuf::Reflection * reflection = f_message.GetReflection();

    if(f_fieldDescriptor->is_repeated())
    {
        int numberOfRepetitions = reflection->FieldSize(f_message, f_fieldDescriptor);
        if(numberOfRepetitions == 0)
        {
            // TODO: remove duplicate code
            result += colorize(ColorClass::VerticalGuides, f_currentPrefix);
            std::string repName;
            repName += colorize(ColorClass::RepeatedFieldName, f_fieldDescriptor->name());
            repName += colorize(ColorClass::RepeatedCount, "[0/0]");
            result += repName;
            size_t nameSize = repName.size();
            result += generateHorizontalGuide(nameSize, maxFieldNameSize);
            result += " = " + colorize(ColorClass::MessageTypeName, "{}");
        }
        if(m_isSimpleMapOutput and f_fieldDescriptor->is_map() and isMapEntryPrimitive(f_fieldDescriptor->message_type()))
        {
            std::map<std::int64_t, const google::protobuf::Message*> int64Map;
            std::map<std::uint64_t, const google::protobuf::Message*> uint64Map;
            std::map<std::string, const google::protobuf::Message*> stringMap;
            for(int i=0; i< numberOfRepetitions; i++)
            {
                if(f_fieldDescriptor->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
                {
                    //using this method to get repeated message from field
                    const google::protobuf::Message & subMessage = reflection->GetRepeatedMessage(f_message, f_fieldDescriptor, i);
                    const google::protobuf::FieldDescriptor * k_fieldDescriptor = f_fieldDescriptor->message_type()->field(0);
                    const google::protobuf::Reflection * reflection = subMessage.GetReflection();
                    switch(k_fieldDescriptor->type())
                    {
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT32:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT32:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
                            {
                                std::int64_t key = static_cast<int64_t>(reflection->GetInt32(subMessage, k_fieldDescriptor));
                                int64Map.insert(std::make_pair(key, &subMessage));
                            }
                            break;
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_SINT64:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_INT64:
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
                            {
                                std::int64_t key = reflection->GetInt64(subMessage, k_fieldDescriptor);
                                int64Map.insert(std::make_pair(key, &subMessage));
                            }
                            break;
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT32:
                            {
                                std::uint64_t key = static_cast<uint64_t>(reflection->GetUInt32(subMessage, k_fieldDescriptor));
                                uint64Map.insert(std::make_pair(key, &subMessage));
                            }
                            break;
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_UINT64:
                            {
                                std::uint64_t key = reflection->GetUInt64(subMessage, k_fieldDescriptor);
                                uint64Map.insert(std::make_pair(key, &subMessage));
                            }
                            break;
                        case grpc::protobuf::FieldDescriptor::Type::TYPE_STRING:
                            {
                                std::string key = reflection->GetString(subMessage, k_fieldDescriptor);
                                stringMap.insert(std::make_pair(key, &subMessage));
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            // first determine which map is not empty, and then output it.
            const google::protobuf::FieldDescriptor * v_fieldDescriptor = f_fieldDescriptor->message_type()->field(1);
            if(!int64Map.empty())
            {
                result += outputMapTitle(int64Map, f_fieldDescriptor, f_currentPrefix);
                for(auto& p: int64Map)
                {
                    result += "\n";
                    result += colorize(ColorClass::VerticalGuides, f_currentPrefix+f_initPrefix);
                    result += stringFromInt(p.first, CustomStringModifier::DecAndHex);
                    result += " => ";
                    result += fieldValueToString(*p.second, v_fieldDescriptor, f_initPrefix, f_currentPrefix, CustomStringModifier::Default);
                }
            }
            if(!uint64Map.empty())
            {
                result += outputMapTitle(uint64Map, f_fieldDescriptor, f_currentPrefix);
                for(auto& p: uint64Map)
                {
                    result += "\n";
                    result += colorize(ColorClass::VerticalGuides, f_currentPrefix+f_initPrefix);
                    result += stringFromUInt(p.first, CustomStringModifier::DecAndHex);
                    result += " => ";
                    result += fieldValueToString(*p.second, v_fieldDescriptor, f_initPrefix, f_currentPrefix, CustomStringModifier::Dec);
                }
            }
            if(!stringMap.empty())
            {
                result += outputMapTitle(stringMap, f_fieldDescriptor, f_currentPrefix);
                for(auto& p: stringMap)
                {
                    result += "\n";
                    result += colorize(ColorClass::VerticalGuides, f_currentPrefix+f_initPrefix);
                    result += stringFromString(p.first, CustomStringModifier::Default);
                    result += " => ";
                    result += fieldValueToString(*p.second, v_fieldDescriptor, f_initPrefix, f_currentPrefix, CustomStringModifier::Default);
                }
            }
            return result;
        }
        for(int i = 0; i < numberOfRepetitions; i++)
        {
            if(i!=0)
            {
                result += "\n";
            }
            result += colorize(ColorClass::VerticalGuides, f_currentPrefix);
            std::string repName;
            repName += getColor(ColorClass::RepeatedFieldName) + f_fieldDescriptor->name() + getColor(ColorClass::Normal);
            repName += getColor(ColorClass::RepeatedCount) + "[" + std::to_string(i+1) + "/" + std::to_string(numberOfRepetitions) + "]" + getColor(ColorClass::Normal);
            result += repName;
            size_t nameSize = repName.size();
            result += generateHorizontalGuide(nameSize, maxFieldNameSize);
            result += " = ";
            result += repeatedFieldValueToString(f_message, f_fieldDescriptor, f_initPrefix, f_currentPrefix, i);
        }

    }
    else
    {
        result += colorize(ColorClass::VerticalGuides, f_currentPrefix);
        result += colorize(ColorClass::NonRepeatedFieldName, f_fieldDescriptor->name());
        size_t nameSize = f_fieldDescriptor->name().size();
        result += generateHorizontalGuide(nameSize, maxFieldNameSize);
        result += " = ";
        result += fieldValueToString(f_message, f_fieldDescriptor, f_initPrefix, f_currentPrefix);
    }

    return result;
}

std::string OutputFormatter::messageToString(const grpc::protobuf::Message & f_message, const grpc::protobuf::Descriptor* f_messageDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix)
{
    std::string result;
    // first determine field name length maximum (for aligned formatting)
    size_t maxFieldNameLength = 0;
    for(int i = 0; i< f_messageDescriptor->field_count(); i++)
    {
        const google::protobuf::FieldDescriptor * fieldDesc = f_messageDescriptor->field(i);
        size_t thisFieldNameLength = fieldDesc->name().size();

        if(fieldDesc->is_repeated())
        {
            const google::protobuf::Reflection * reflection = f_message.GetReflection();
            int numberOfRepetitions = reflection->FieldSize(f_message, fieldDesc);
            std::string simulatedMaxArr =  "[" + std::to_string(numberOfRepetitions) + "/" + std::to_string(numberOfRepetitions) + "]";
            thisFieldNameLength += simulatedMaxArr.size();
            
            
        }
        if(thisFieldNameLength > maxFieldNameLength)
        {
            maxFieldNameLength = thisFieldNameLength;
        }
    }

    for(int i = 0; i< f_messageDescriptor->field_count(); i++)
    {
        const google::protobuf::FieldDescriptor * fieldDesc = f_messageDescriptor->field(i);
        if(i!=0)
        {
            result += "\n";
        }
        result += fieldToString(f_message, fieldDesc, f_initPrefix, f_currentPrefix, maxFieldNameLength);
    }
    return result;
}

std::string OutputFormatter::stringFromBytes(const std::string & f_value, const CustomStringModifier & f_modifier, const std::string & f_prefix)
{
    std::string result;

    if(f_modifier == CustomStringModifier::Raw)
    {
        return f_value;
    }
    else // Default, Hex, Dec
    {
        // a simple hexdump:
        const std::string & prefix = f_prefix;
        result += "hex[" + std::to_string(f_value.size()) + "]";

        std::string stringRepresentation = "";
        size_t maxAddrTextSize = std::to_string(f_value.size()-1).size();
        for(size_t i = 0; i<f_value.size(); )
        {
            // first decide on linebreaks, prefix etc:
            if(i%8 == 0)
            {
                if(f_value.size() > 8)
                {
                    result += "\n" + prefix;
                    std::stringstream streamAddr;
                    streamAddr << std::setfill (' ') << std::setw(maxAddrTextSize);
                    // TODO: should place address as hex also...
                    streamAddr << i;
                    result += streamAddr.str();
                    result += ": ";
                }
                else
                {
                    result += " = ";
                }
            }
            else if(i%4 == 0)
            {
                result += "  ";
            }
            else
            {
                result += " ";
            }

            // now do the actual hexdump:
            // cast to uint16_t necessary, otherwise stream interprets the value as a char.
            uint16_t byteVal = f_value[i] & 0xff;
            std::stringstream stream;
            stream << std::setfill ('0') << std::setw(2) << std::hex;
            stream << byteVal;
            result += stream.str();

            // create string representation:
            if( (f_value[i] >= 32) and (f_value[i] <= 126) )
            {
                // string representable character range:
                stringRepresentation += f_value[i];
            }
            else
            {
                // special characters
                stringRepresentation += ".";
            }

            i++;

            // Place string representation, when appropriate:
            if( (i%8 == 0) or (i>=f_value.size()) )
            {
                size_t padding = (8 - i%8)%8;
                if(padding>3)
                {
                    padding *=3;
                    padding += 1;
                }
                else
                {
                    padding *=3;
                }
                result += std::string(padding, ' ');
                result += " |" + stringRepresentation + "|";
                stringRepresentation = "";
            }
        }
    }
    return result;
}
}
