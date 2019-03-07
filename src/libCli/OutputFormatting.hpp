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

#pragma once
#include <third_party/gRPC_utils/proto_reflection_descriptor_database.h>

#include <sstream>
#include <iomanip>

namespace cli
{
    /// Class with methods to format a protobuf message into human readable strings.
    class OutputFormatter
    {
        public:
            /// Initializes the OutputFormatter with default settings.
            /// Includes initialization of the default color theme.
            OutputFormatter();

            /// Enum containing all possible elements for colorization.
            enum class ColorClass
            {
                Normal,                 // Used for all text not covered by a color class
                NonRepeatedFieldName,   // Field name of non-repeated fields
                RepeatedFieldName,      // Field name of repeated fields
                VerticalGuides,         // The indentation string
                HorizontalGuides,       // The filling string (dots) from fieldname to value
                RepeatedCount,          // Counter (both: current and total) of repeated fields
                BoolTrue,               // true value of bools
                BoolFalse,              // false value of bools
                StringValue,            // string values
                MessageTypeName,        // type name of messages
                DecimalValue,           // devimal values of numbers
                HexValue,               // hex value of numbers
                EnumValue               // enum values
            };

            /// Possible modifiers, which may be used to control how the formatter converts certain types into string.
            enum class CustomStringModifier
            {
                None, // does not change behavior
                Raw   // Prints integers in hex and without colorization. TODO: better name
            };

            /// Formats a protobuf message into a human readable string.
            /// @param f_message the protobuf message to be formatted
            /// @param f_messageDescriptor descriptor describing the message type
            /// @param f_initPrefix Each new line in formatted output will be prefixed with 0 or more instances of this string corresponding to the indentation level.
            /// @param f_currentPrefix Each new line in formatted output will be prefixed with this sting. May be used for an initial prefix/indentation of output text.
            std::string messageToString(
                    const grpc::protobuf::Message & f_message,
                    const grpc::protobuf::Descriptor* f_messageDescriptor,
                    const std::string & f_initPrefix = " ",
                    const std::string & f_currentPrefix = ""
                    );

            /// Clears the color map.
            /// Causes all output to be generated with default font (no terminal control characters).
            void clearColorMap();

            // TODO: provide the option to provide custom color map
            //       e.g. via config file or cli args

            /// Formats a field value as string.
            /// NOTE: required for custom output format
            std::string fieldValueToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, CustomStringModifier f_modifier = CustomStringModifier::None);

            /// Formats a repeated field value as string.
            /// NOTE: required for custom output format
            std::string repeatedFieldValueToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, int f_fieldIndex, CustomStringModifier f_modifier = CustomStringModifier::None);

        private:
            std::map<ColorClass, std::string> m_colorMap;
            std::string generateHorizontalGuide(size_t f_currentSize, size_t f_targetSize);
            std::string getColor(ColorClass f_colorClass);
            std::string colorize(ColorClass f_colorClass, const std::string & f_string);
            std::string fieldToString(const grpc::protobuf::Message & f_message, const google::protobuf::FieldDescriptor * f_fieldDescriptor, const std::string & f_initPrefix, const std::string & f_currentPrefix, size_t maxFieldNameSize);
            template <typename T> std::string intToHexString(T f_value);

            // string formatting methods for various types:
            template<typename T>
                std::string stringFromInt(T f_value, const CustomStringModifier & f_modifier)
                {
                    std::string result;
                    if(f_modifier == CustomStringModifier::Raw)
                    {
                        result += intToHexString(f_value);
                    }
                    else
                    {
                        result += colorize(ColorClass::DecimalValue, std::to_string(f_value));
                    }
                    return result;
                }

            template<typename T>
                std::string stringFromUInt(T f_value, const CustomStringModifier & f_modifier)
                {
                    std::string result;
                    if(f_modifier == CustomStringModifier::Raw)
                    {
                        result += intToHexString(f_value);
                    }
                    else
                    {
                        std::stringstream stream;
                        stream << colorize(ColorClass::DecimalValue, std::to_string(f_value))
                            << " (" << intToHexString(f_value) << ")"
                            << getColor(ColorClass::Normal);
                        result += stream.str();
                    }
                    return result;
                }

            template<typename T>
                std::string stringFromFloat(T f_value, const CustomStringModifier & f_modifier)
                {
                    return colorize(ColorClass::DecimalValue, std::to_string(f_value));
                }

            std::string stringFromBool(bool f_value, const CustomStringModifier & f_modifier)
            {
                return (f_value ? colorize(ColorClass::BoolTrue,"true") : colorize(ColorClass::BoolFalse,"false"));
            }

            std::string stringFromString(const std::string & f_value, const CustomStringModifier & f_modifier)
            {
                return colorize(ColorClass::StringValue, "\"" + f_value + "\"");
            }

            std::string stringFromEnum(const google::protobuf::EnumValueDescriptor * f_value, const CustomStringModifier & f_modifier)
            {
                return colorize(ColorClass::EnumValue, f_value->name());
            }

            std::string stringFromBytes(const std::string & f_value, const CustomStringModifier & f_modifier, const std::string & f_prefix);

    };
}
