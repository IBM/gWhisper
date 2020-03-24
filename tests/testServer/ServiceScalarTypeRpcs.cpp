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

#include "ServiceScalarTypeRpcs.hpp"
#include <algorithm>
#include <string>

::grpc::Status ServiceScalarTypeRpcs::incrementNumbers(
        ::grpc::ServerContext* context,
        const ::examples::Numbers* request,
        ::examples::Numbers* response
        )
{
    response->set_m_double(request->m_double() + 1.0);
    response->set_m_float(request->m_float() + 1.0);
    response->set_m_int32(request->m_int32() + 1);
    response->set_m_uint32(request->m_uint32() + 1);
    response->set_m_int64(request->m_int64() + 1);
    response->set_m_uint64(request->m_uint64() + 1);
    return grpc::Status();
}

::grpc::Status ServiceScalarTypeRpcs::negateBool(
        ::grpc::ServerContext* context,
        const ::examples::Bool* request,
        ::examples::Bool* response
        )
{
    response->set_m_bool(not request->m_bool());
    return grpc::Status();
}

::grpc::Status ServiceScalarTypeRpcs::capitalizeString(
        ::grpc::ServerContext* context,
        const ::examples::String* request,
        ::examples::String* response
        )
{
    const std::string & str = request->text();
    std::string result;
    for(char c : request->text())
    {
        result += ::toupper(c);
    }
    response->set_text(result);
    return grpc::Status();
}

::grpc::Status ServiceScalarTypeRpcs::bitwiseInvertBytes(
        ::grpc::ServerContext* context,
        const ::examples::Bytes* request,
        ::examples::Bytes* response
        )
{
    for(char byteChar : request->data())
    {
        (*(response->mutable_data())) += (~byteChar);
    }
    return grpc::Status();
}
