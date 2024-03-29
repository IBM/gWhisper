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

#include <libArgParse/ArgParse.hpp>
#include <google/protobuf/dynamic_message.h>

namespace cli
{

    class MessageParser
    {
        public:
            /// Constructs gRPC messages from a given parseTree.
            /// From a given parse tree, this function constructs a protobuf message and fills
            /// it with all data available from the parse tree.
            /// @param f_parseTree The parse tree containing all information which should be "parsed" into the protobuf messages.
            /// @param f_factory Required to construct messages.
            /// @param f_messageDescriptor Message descriptor describing the type of the messache whoch should be constructed.
            /// @returns unique_ptr pointing to a newly created message if parse succedded,
            ///      or an unassociated unique_ptr if parse failed.
            virtual std::vector<std::unique_ptr<google::protobuf::Message>> parseMessages(
                ArgParse::ParsedElement & f_parseTree,
                google::protobuf::DynamicMessageFactory & f_factory,
                const google::protobuf::Descriptor* f_messageDescriptor,
                bool f_isClientStreamingRpc
                ) = 0;
    };

    class MessageParserCli : public MessageParser
    {
        public:
            virtual std::vector<std::unique_ptr<google::protobuf::Message>> parseMessages(
                ArgParse::ParsedElement & f_parseTree,
                google::protobuf::DynamicMessageFactory & f_factory,
                const google::protobuf::Descriptor* f_messageDescriptor,
                bool f_isClientStreamingRpc
                ) override;
        private:
            std::unique_ptr<google::protobuf::Message> parseMessage(
                    ArgParse::ParsedElement & f_parseTree,
                    google::protobuf::DynamicMessageFactory & f_factory,
                    const google::protobuf::Descriptor* f_messageDescriptor
                    );
            int parseFieldValue(ArgParse::ParsedElement & f_parseTree, google::protobuf::Message * f_message, google::protobuf::DynamicMessageFactory & f_factory, const google::protobuf::FieldDescriptor * f_fieldDescriptor, bool f_isRepeated = false);
    };

    class MessageParserJson : public MessageParser
    {
        public:
            virtual std::vector<std::unique_ptr<google::protobuf::Message>> parseMessages(
                ArgParse::ParsedElement & f_parseTree,
                google::protobuf::DynamicMessageFactory & f_factory,
                const google::protobuf::Descriptor* f_messageDescriptor,
                bool f_isClientStreamingRpc
                ) override;
        private:
            std::unique_ptr<google::protobuf::Message> parseMessage(
                    ArgParse::ParsedElement & f_parseTree,
                    google::protobuf::DynamicMessageFactory & f_factory,
                    const google::protobuf::Descriptor* f_messageDescriptor
                    );
    };
}
