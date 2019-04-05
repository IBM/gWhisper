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

class ServiceScalarTypeRpcs final : public examples::ScalarTypeRpcs::Service
{
    virtual  ::grpc::Status incrementNumbers(
            ::grpc::ServerContext* context,
            const ::examples::Numbers* request,
            ::examples::Numbers* response
            ) override;

    virtual  ::grpc::Status negateBool(
            ::grpc::ServerContext* context,
            const ::examples::Bool* request,
            ::examples::Bool* response
            ) override;

    virtual  ::grpc::Status capitalizeString(
            ::grpc::ServerContext* context,
            const ::examples::String* request,
            ::examples::String* response
            ) override;

    virtual  ::grpc::Status bitwiseInvertBytes(
            ::grpc::ServerContext* context,
            const ::examples::Bytes* request,
            ::examples::Bytes* response
            ) override;
};
