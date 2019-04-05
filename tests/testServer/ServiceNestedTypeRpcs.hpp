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

class ServiceNestedTypeRpcs final : public examples::NestedTypeRpcs::Service
{
    virtual  ::grpc::Status duplicateEverything1d(
            ::grpc::ServerContext* context,
            const ::examples::NestedMessage1d* request,
            ::examples::NestedMessage1d* response
            ) override;
    virtual  ::grpc::Status duplicateEverything2d(
            ::grpc::ServerContext* context,
            const ::examples::NestedMessage2d* request,
            ::examples::NestedMessage2d* response
            ) override;
    virtual  ::grpc::Status nestedEmpty(
            ::grpc::ServerContext* context,
            const ::examples::NestedEmpty* request,
            ::google::protobuf::Empty* response
            ) override;
    virtual  ::grpc::Status getTime(
            ::grpc::ServerContext* context,
            const ::google::protobuf::Empty* request,
            ::google::protobuf::Timestamp* response
            ) override;
};
