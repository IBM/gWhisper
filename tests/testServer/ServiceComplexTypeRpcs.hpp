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

class ServiceComplexTypeRpcs final : public examples::ComplexTypeRpcs::Service
{
    virtual  ::grpc::Status echoColorEnum(
            ::grpc::ServerContext* context,
            const ::examples::ColorEnum* request,
            ::examples::ColorEnum* response
            ) override;

    virtual  ::grpc::Status getNumberOrStringOneOf(
            ::grpc::ServerContext* context,
            const ::examples::NumberOrStringChoice* request,
            ::examples::NumberOrStringOneOf* response
            ) override;

    virtual  ::grpc::Status sendNumberOrStringOneOf(
            ::grpc::ServerContext* context,
            const ::examples::NumberOrStringOneOf* request,
            ::examples::NumberOrStringChoice* response
            ) override;

    virtual  ::grpc::Status addAllNumbers(
            ::grpc::ServerContext* context,
            const ::examples::RepeatedNumbers* request,
            ::examples::Uint32* response
            ) override;

    virtual  ::grpc::Status getLastColor(
            ::grpc::ServerContext* context,
            const ::examples::RepeatedColors* request,
            ::examples::ColorEnum* response
            ) override;

    virtual  ::grpc::Status echoNumberAndStrings(
            ::grpc::ServerContext* context,
            const ::examples::RepeatedNumberAndString* request,
            ::examples::RepeatedNumberAndString* response
            ) override;

    virtual  ::grpc::Status mapNumbersToString(
            ::grpc::ServerContext* context,
            const ::examples::RepeatedNumbers* request,
            ::examples::NumberMap* response
            ) override;
};
