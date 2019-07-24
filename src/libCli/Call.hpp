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
#include <libArgParse/ArgParse.hpp>

namespace cli
{
    /// Performs an RPC call based on information from the parse tree.
    /// @param f_parseTree Parse tree containing all relevant information for the call (server address, request message, options, ...).
    /// @returns 0 if RPC succeeded, -1 otherwise (including parse errors from parse tree and gRPC bad return code)
    int call(ArgParse::ParsedElement & f_parseTree);

    class ChannelManager
    {
        public:
            ChannelManager() = delete;
        public:
            static std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, std::string f_serverPort)
            {
                static std::shared_ptr<grpc::Channel> m_channel;
                if(f_serverPort == "")
                {
                    f_serverPort = "50051";
                }
                f_serverAddress += ":" + f_serverPort;

                if(m_channel == nullptr)
                {
                    m_channel = grpc::CreateChannel(f_serverAddress, grpc::InsecureChannelCredentials());
                }
                return m_channel;
            };
    };
}
