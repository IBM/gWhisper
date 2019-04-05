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

#include "examples.grpc.pb.h"

class ServiceStreamingRpcs final : public examples::StreamingRpcs::Service
{
    virtual  ::grpc::Status replyStreamEmpty(
            ::grpc::ServerContext* context,
            const ::examples::Uint32* request,
            ::grpc::ServerWriter< ::google::protobuf::Empty>* writer
            ) override;

    virtual  ::grpc::Status replyStreamTimestamp10Hz(
            ::grpc::ServerContext* context,
            const ::examples::Uint32* request,
            ::grpc::ServerWriter< ::google::protobuf::Timestamp>* writer
            ) override;

    virtual  ::grpc::Status requestStreamAddAllNumbers(
            ::grpc::ServerContext* context,
            ::grpc::ServerReader< ::examples::Uint32>* reader,
            ::examples::Uint32* response
            ) override;

    virtual  ::grpc::Status requestStreamCountMessages(
            ::grpc::ServerContext* context,
            ::grpc::ServerReader< ::google::protobuf::Empty>* reader,
            ::examples::Uint32* response
            ) override;

    virtual  ::grpc::Status bidirectionalStreamNegateNumbers(
            ::grpc::ServerContext* context,
            ::grpc::ServerReaderWriter< ::examples::Int32,
            ::examples::Int32>* stream
            ) override;
};
