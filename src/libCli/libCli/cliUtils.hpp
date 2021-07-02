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
#include "libArgParse/ArgParse.hpp"
#include <grpc++/channel.h>
namespace cli
{
    /// Wait for a gRPC channel to go into connected state.
    /// @param f_channel the channel to be waited on
    /// @param f_timeoutMs Timeout in milliseconds
    /// @returns true if channel is connected, false if timeout exceeded and channel is still not in connected state.
    bool waitForChannelConnected(std::shared_ptr<grpc::Channel> f_channel, uint32_t f_timeoutMs);

    /// Retrieves the "connectTimeout" option from the parse tree
    /// @param f_parseTree Parse-tree which should be searched for the option
    /// @param f_default default value returned, if parse-tree did not contain the option.
    /// @returns the value as an integer
    uint32_t getConnectTimeoutMs(ArgParse::ParsedElement * f_parseTree, uint32_t f_default = 500);

    /// Convert a gRPC status code into a string.
    /// @param f_statusCode The status code to convert.
    /// @returns a string representation if one was found. Empty string otherwise.
    std::string getGrpcStatusCodeAsString(grpc::StatusCode f_statusCode);

}
