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
#include <mutex>
#include <thread>

static size_t rpcCount = 0;
std::mutex rpcCountMutex;


::grpc::Status ServiceStatusHandling::neverEndingRpc(
        ::grpc::ServerContext* context,
        const ::google::protobuf::Empty* request,
        ::google::protobuf::Empty* response
        )
{
    rpcCountMutex.lock();
    rpcCount++;
    size_t rpcCountCached = rpcCount;
    rpcCountMutex.unlock();

    std::thread::id threadId = std::this_thread::get_id();

    std::cout << threadId << " neverEndingRpc started! Currently " << rpcCountCached << " never ending RPCs are running" << std::endl;

    uint64_t i = 0;
    while(not context->IsCancelled())
    {
        sleep(1);
        i++;
        //std::cout << "  " << threadId << " neverEndingRpc running for " << std::to_string(i) << "seconds" << std::endl;
    }

    rpcCountMutex.lock();
    rpcCount--;
    rpcCountCached=rpcCount;
    rpcCountMutex.unlock();

    std::cout << threadId << " neverEndingRpc was cancelled! It ran for " << i << " seconds. Currently " << rpcCountCached << " never ending RPCs are running" << std::endl;
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
