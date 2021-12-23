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

#include <libCli/MessageFormatter.hpp>
#include <google/protobuf/util/json_util.h>

namespace cli
{
    MessageFormatterCustom::MessageFormatterCustom(ArgParse::ParsedElement & f_parseTree) :
        m_parseTree(f_parseTree)
    {
    }

    std::string MessageFormatterCustom::messageToString(
            const grpc::protobuf::Message & f_message,
            const grpc::protobuf::Descriptor* f_messageDescriptor)
    {
        bool customOutputFormatRequested = false;
        ArgParse::ParsedElement customFormatParseTree = m_parseTree.findFirstSubTree("CustomOutputFormat", customOutputFormatRequested);

        std::string resultString;
        if(not customOutputFormatRequested)
        {
            resultString = "Warning: Requested to use custom message formatter, but no format given.";
            return resultString;
        }

        resultString = customMessageFormat(f_message, f_messageDescriptor, customFormatParseTree);

        return resultString;
    }

    std::string MessageFormatterCustom::customMessageFormat(const grpc::protobuf::Message &f_message, const grpc::protobuf::Descriptor *f_messageDescriptor, ArgParse::ParsedElement &f_customFormatParseTree, size_t startChild)
    {
        std::string result;
        const google::protobuf::Reflection *reflection = f_message.GetReflection();

        // first look for target fields to format:
        bool found = false;
        ArgParse::ParsedElement targetList = f_customFormatParseTree.findFirstSubTree("TargetSpecifier", found);

        if ((targetList.getChildren().size() > startChild) && (targetList.getChildren()[startChild] != nullptr))
        {
            std::string partialTarget = targetList.getChildren()[startChild]->findFirstChild("PartialTarget");
            //std::cout << "looking at '" << partialTarget << "'\n";
            if (partialTarget != "")
            {
                // empty target addresses the current message
                const google::protobuf::FieldDescriptor *partialField = f_messageDescriptor->FindFieldByName(partialTarget);
                if (partialField == nullptr)
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

                if (partialField->is_repeated())
                {
                    //std::cout << "have repeated\n";
                    switch (partialField->type())
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
                    for (int j = 0; j < numberOfRepetitions; j++)
                    {
                        //std::cout << " have repeated entry\n";
                        const google::protobuf::Message &subMessage = reflection->GetRepeatedMessage(f_message, partialField, j);
                        result += customMessageFormat(subMessage, partialField->message_type(), f_customFormatParseTree, startChild + 1);
                    }
                    return result;
                }
                if (partialField->type() == grpc::protobuf::FieldDescriptor::Type::TYPE_MESSAGE)
                {
                    //std::cout << "have message\n";
                    const google::protobuf::Message &subMessage = reflection->GetMessage(f_message, partialField);
                    return customMessageFormat(subMessage, partialField->message_type(), f_customFormatParseTree, startChild + 1);
                }
            }
        }

        // now we know we are not repeated and now f_message contains the correct
        // context in which to evaluate field references :)

        //std::cout << "have field\n";
        MessageFormatterOptimizedForHumans myMessageFormatter;
        myMessageFormatter.clearColorMap();

        bool haveFormatString = false;
        auto formatString = f_customFormatParseTree.findFirstSubTree("OutputFormatString", haveFormatString);
        if (not haveFormatString)
        {
            return "Error: no format string given\n";
        }
        for (auto outputStatement : formatString.getChildren())
        {
            bool foundFieldReference = false;
            auto fieldReference = outputStatement->findFirstSubTree("OutputFieldReference", foundFieldReference);
            if (foundFieldReference)
            {
                MessageFormatterOptimizedForHumans::CustomStringModifier modifier = MessageFormatterOptimizedForHumans::getModifier(*outputStatement);

                //std::cout << "  have field ref " <<  fieldReference.getMatchedString() << "\n";
                // need to lookup the field:
                const google::protobuf::FieldDescriptor *fieldRef = f_messageDescriptor->FindFieldByName(fieldReference.getMatchedString());
                if (fieldRef == nullptr)
                {
                    result += "???";
                }
                else
                {
                    result += myMessageFormatter.fieldValueToString(f_message, fieldRef, "", "", modifier);
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

}
