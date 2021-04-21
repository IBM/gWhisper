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

    if (argc >= 2 and (std::string(argv[1]) == "-h" or std::string(argv[1]) == "--help"))
    {
        std::cout << "A simple gRPC test server implementing RPCs using most of the proto3 language features." << std::endl
                  << std::endl;
        std::cout << "SYNOPSIS:" << std::endl;
        std::cout << "testServer [OPTIONS] [PORT]" << std::endl
                  << std::endl;
        std::cout << "OPTIONS:" << std::endl;
        std::cout << "  -h" << std::endl;
        std::cout << "  --help" << std::endl;
        std::cout << "     Shows this help" << std::endl
                  << std::endl;
        std::cout << "PORT:" << std::endl;
        std::cout << "  The TCP port the server should listen to." << std::endl;
        std::cout << "  Default: 50051" << std::endl;
        return 0;
    }
    std::string serverAddr = "0.0.0.0:50051";
    if (argc >= 2)
    {
        serverAddr = "0.0.0.0:" + std::string(argv[1]);
    }

    // Frage: Wo wird der Channel geöffnet? Hier create ich doch nur einen Insecure Server
    // Wie enable ich TLS für den Server? Ich habe da keinen Parameter gefunden. Genügt es über den TLS Port zu gehen?:
    // std::shared_ptr< grpc::SslServerCredentials > secureCredentials
    // builder.AddListeningPort(addr+":465", creds )

    std::cout << "Starting server listening on " << serverAddr << std::endl;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());

    std::cout << "Starting secure server listening on " << serverAddr << std::endl;
    // Create a default SSL Credentials object.
    std::shared_ptr<grpc::ServerCredentials> creds;

    // Here we can change to chain of trust instead of selfsigned
    grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {};
    // TODO: Set the right options
    creds = grpc::ServerCredentials(grpc::SslCredentialsOptions());
    builder.AddListeningPort(serverAddr, creds);

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
    if (server != nullptr)
    {
        server->Wait();
    }
    else
    {
        std::cout << "Server failed to start. exiting." << std::endl;
        return -1;
    }
    return 0;
}
