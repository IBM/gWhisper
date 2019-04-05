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

#include "ServiceComplexTypeRpcs.hpp"

::grpc::Status ServiceComplexTypeRpcs::echoColorEnum(
        ::grpc::ServerContext* context,
        const ::examples::ColorEnum* request,
        ::examples::ColorEnum* response
        )
{
    response->set_color(request->color());
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::getNumberOrStringOneOf(
        ::grpc::ServerContext* context,
        const ::examples::NumberOrStringChoice* request,
        ::examples::NumberOrStringOneOf* response
        )
{
    switch(request->choice())
    {
        case ::examples::NumberOrStringChoice_Choice_number:
            response->set_number(42);
            break;
        case ::examples::NumberOrStringChoice_Choice_str:
            response->set_str("forty two");
            break;
        case ::examples::NumberOrStringChoice_Choice_both:
            response->mutable_both()->set_number(42);
            response->mutable_both()->set_str("forty two");
            break;
        default:
            return grpc::Status(grpc::StatusCode::ABORTED, "Invalid enum value");

    }
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::sendNumberOrStringOneOf(
        ::grpc::ServerContext* context,
        const ::examples::NumberOrStringOneOf* request,
        ::examples::NumberOrStringChoice* response
        )
{
    switch(request->number_or_string_case())
    {
        case ::examples::NumberOrStringOneOf::kNumber:
            response->set_choice(::examples::NumberOrStringChoice_Choice_number);
            break;
        case ::examples::NumberOrStringOneOf::kStr:
            response->set_choice(::examples::NumberOrStringChoice_Choice_str);
            break;
        case ::examples::NumberOrStringOneOf::kBoth:
            response->set_choice(::examples::NumberOrStringChoice_Choice_both);
            break;
        default:
            return grpc::Status(grpc::StatusCode::ABORTED, "Oneof has nothing");
    }
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::addAllNumbers(
        ::grpc::ServerContext* context,
        const ::examples::RepeatedNumbers* request,
        ::examples::Uint32* response
        )
{
    uint32_t result = 0;
    for(uint32_t num : request->numbers())
    {
        result += num;
    }
    response->set_number(result);
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::getLastColor(
        ::grpc::ServerContext* context,
        const ::examples::RepeatedColors* request,
        ::examples::ColorEnum* response
        )
{
    if(request->colors_size() > 0)
    {
        response->set_color(request->colors(request->colors_size()-1));
    }
    else
    {
        return grpc::Status(grpc::StatusCode::ABORTED, "No colors were given.");
    }
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::echoNumberAndStrings(
        ::grpc::ServerContext* context,
        const ::examples::RepeatedNumberAndString* request,
        ::examples::RepeatedNumberAndString* response
        )
{
    for(auto nas : request->number_and_strings())
    {
        auto newElement = response->add_number_and_strings();
        newElement->set_str(nas.str());
        newElement->set_number(nas.number());
    }
    return grpc::Status();
}

::grpc::Status ServiceComplexTypeRpcs::mapNumbersToString(
        ::grpc::ServerContext* context,
        const ::examples::RepeatedNumbers* request,
        ::examples::NumberMap* response
        )
{
    for(auto num : request->numbers())
    {
        response->mutable_number_to_string()->operator[](num) = std::to_string(num);
    }
    return grpc::Status();
}
