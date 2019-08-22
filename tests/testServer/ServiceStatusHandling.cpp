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

#include "ServiceStatusHandling.hpp"
#include <unistd.h>

::grpc::Status ServiceStatusHandling::neverEndingRpc(
        ::grpc::ServerContext* context,
        const ::google::protobuf::Empty* request,
        ::google::protobuf::Empty* response
        )
{
    uint64_t i = 0;
    while(true)
    {
        sleep(1);
        i++;
        std::cout << "neverEndingRpc running for " << std::to_string(i) << "seconds" << std::endl;
    }
    return grpc::Status();
}

::grpc::Status ServiceStatusHandling::giveStatusAborted(
        ::grpc::ServerContext* context,
        const ::google::protobuf::Empty* request,
        ::google::protobuf::Empty* response
        )
{
    return grpc::Status(grpc::StatusCode::ABORTED, "Call was aborted as intended by this example.");
}
