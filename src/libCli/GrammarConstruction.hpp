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
#include <third_party/gRPC_utils/proto_reflection_descriptor_database.h>

namespace cli
{
    /// Constructs the grammar for the gWhisper CLI.
    /// @param f_grammarPool Pool to allocate grammar elements from.
    /// @returns the root element of the generated grammar. The pointer should not
    ///          be used after the given f_grammarPool is de-allocated.
    ArgParse::GrammarElement * constructGrammar(ArgParse::Grammar & f_grammarPool);
    class MessageGrammarFactory{
      public: 
      MessageGrammarFactory(ArgParse::Grammar& grammar):
        m_grammar(grammar){
        }
        ArgParse::GrammarElement * getMessageGrammar(const std::string & f_rootElementName, const grpc::protobuf::Descriptor* f_messageDescriptor, ArgParse::GrammarElement * f_wrappingElement = nullptr);

      private:
        void addFieldValueGrammar(ArgParse::GrammarElement * f_fieldGrammar, const grpc::protobuf::FieldDescriptor * f_field);
        ArgParse::Grammar & m_grammar;
    };

}
