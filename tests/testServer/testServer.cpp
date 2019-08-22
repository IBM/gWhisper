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

#include <iostream>

#include <grpcpp/grpcpp.h>
#include "ServiceScalarTypeRpcs.hpp"
#include "ServiceStreamingRpcs.hpp"
#include "ServiceComplexTypeRpcs.hpp"
#include "ServiceNestedTypeRpcs.hpp"
#include "ServiceStatusHandling.hpp"


int main(int argc, char **argv)
{

    if(argc >= 2 and (std::string(argv[1]) == "-h" or std::string(argv[1]) == "--help"))
    {
        std::cout << "A simple gRPC test server implementing RPCs using most of the proto3 language features." << std::endl << std::endl;
        std::cout << "SYNOPSIS:" << std::endl;
        std::cout << "testServer [OPTIONS] [PORT]" << std::endl << std::endl;
        std::cout << "OPTIONS:" << std::endl;
        std::cout << "  -h" << std::endl;
        std::cout << "  --help" << std::endl;
        std::cout << "     Shows this help" << std::endl << std::endl;
        std::cout << "PORT:" << std::endl;
        std::cout << "  The TCP port the server should listen to." << std::endl;
        std::cout << "  Default: 50051" << std::endl;
        return 0;
    }
    std::string addr = "0.0.0.0:50051";
    if(argc >=2)
    {
        addr = "0.0.0.0:" + std::string(argv[1]);
    }
    std::cout << "Starting server listening on " << addr << std::endl;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());

    // register all services:
    ServiceScalarTypeRpcs scalarTypeRpcs;
    builder.RegisterService(&scalarTypeRpcs);

    ServiceNestedTypeRpcs nestedTypeRpcs;
    builder.RegisterService(&nestedTypeRpcs);

    ServiceComplexTypeRpcs complexTypeRpcs;
    builder.RegisterService(&complexTypeRpcs);

    ServiceStreamingRpcs streamingRpcs;
    builder.RegisterService(&streamingRpcs);

    ServiceStatusHandling statusHandling;
    builder.RegisterService(&statusHandling);


    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
    return 0;
}
