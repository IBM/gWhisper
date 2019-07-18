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

#include "ServiceNestedTypeRpcs.hpp"

::grpc::Status ServiceNestedTypeRpcs::duplicateEverything1d(
        ::grpc::ServerContext* context,
        const ::examples::NestedMessage1d* request,
        ::examples::NestedMessage1d* response
        )
{
    response->mutable_some_numbers()->set_m_double(request->some_numbers().m_double()*2);
    response->mutable_some_numbers()->set_m_float(request->some_numbers().m_float()*2);
    response->mutable_some_numbers()->set_m_int32(request->some_numbers().m_int32()*2);
    response->mutable_some_numbers()->set_m_uint32(request->some_numbers().m_uint32()*2);
    response->mutable_some_numbers()->set_m_int64(request->some_numbers().m_int64()*2);
    response->mutable_some_numbers()->set_m_uint64(request->some_numbers().m_uint64()*2);
    response->mutable_number_and_string()->set_number(request->number_and_string().number()*2);
    response->mutable_number_and_string()->set_str(request->number_and_string().str() + request->number_and_string().str());
    response->set_str(request->str());
    return grpc::Status();
}

::grpc::Status ServiceNestedTypeRpcs::duplicateEverything2d(
        ::grpc::ServerContext* context,
        const ::examples::NestedMessage2d* request,
        ::examples::NestedMessage2d* response
        )
{
    duplicateEverything1d(context, &request->sub_tree(), response->mutable_sub_tree());
    response->mutable_number_and_string()->set_number(request->number_and_string().number()*2);
    response->mutable_number_and_string()->set_str(request->number_and_string().str() + request->number_and_string().str());
    response->set_str(request->str());
    return grpc::Status();
}

::grpc::Status ServiceNestedTypeRpcs::nestedEmpty(
        ::grpc::ServerContext* context,
        const ::examples::NestedEmpty* request,
        ::google::protobuf::Empty* response
        )
{
    return grpc::Status();
}

::grpc::Status ServiceNestedTypeRpcs::getTime(
        ::grpc::ServerContext* context,
        const ::google::protobuf::Empty* request,
        ::google::protobuf::Timestamp* response
        )
{
    std::time_t t = std::time(0) ;
    struct tm zero = {0};

    zero.tm_hour    = 0;
    zero.tm_min     = 0;
    zero.tm_sec     = 0;
    zero.tm_year    = 0;
    zero.tm_mon     = 0;
    zero.tm_mday    = 0;

    double seconds = difftime(t, mktime(&zero));

    response->set_seconds(seconds);
    response->set_nanos((seconds- static_cast<uint64_t>(seconds))*1000000000);
    return grpc::Status();
}

::grpc::Status ServiceNestedTypeRpcs::echoNestedMaps(
        ::grpc::ServerContext* context,
        const ::examples::NestedMaps* request,
        ::examples::NestedMaps* response
        )
{

    *response = *request;

    return grpc::Status();
}

::grpc::Status ServiceNestedTypeRpcs::echoRecursiveMaps(
        ::grpc::ServerContext* context,
        const ::examples::RecursiveMaps* request,
        ::examples::RecursiveMaps* response
        )
{

    *response = *request;

    return grpc::Status();
}
