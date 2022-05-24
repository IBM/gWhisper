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
#include "utils/gwhisperUtils.hpp"
#include <grpcpp/security/credentials.h>

void printHelp()
{
    std::cout << "A simple gRPC test server implementing RPCs using most of the proto3 language features." << std::endl
        << std::endl;
    std::cout << "SYNOPSIS:" << std::endl;
    std::cout << "testServer [OPTIONS]" << std::endl
        << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "  -h" << std::endl;
    std::cout << "  --help" << std::endl;
    std::cout << "     Shows this help" << std::endl;
    std::cout << "  --basePort BASE_PORT" << std::endl;
    std::cout << "  --certBasePath CERT_BASE_PATH" << std::endl
        << std::endl;
    std::cout << "CERT_BASE_PATH:" << std::endl;
    std::cout << "  Keys/ certs are expected to be located in this path with" << std::endl;
    std::cout << "  the following naming:" << std::endl;
    std::cout << "     server_key.pem" << std::endl;
    std::cout << "     server_crt.pem" << std::endl;
    std::cout << "     client_crt.pem" << std::endl;
    std::cout << "  Example certs are generated by cert-key-gen.sh and located at build/cert-key-pair/" << std::endl;
    std::cout << "  If no path is given only insecure channel is opened." << std::endl << std::endl;
    std::cout << "BASE_PORT:" << std::endl;
    std::cout << "  The ports used to generate ports for the secure and inscure servers." << std::endl;
    std::cout << "  Default: 50000" << std::endl;
    std::cout << "  Example for port generation" << std::endl;
    std::cout << "  BASEPORT : 50000" << std::endl;
    std::cout << "  Port for insecure server: 50000 + 51 = 50051" << std::endl;
    std::cout << "  Port for secure server without client cert: 50000 + 52 = 50052" << std::endl;
    std::cout << "  Port for secure server with client cert-key pair: 50000 + 443 = 50443" << std::endl;
}

int main(int argc, char **argv)
{
    grpc::EnableDefaultHealthCheckService(true);
    std::string basePort = "50000";
    std::string certBasePath = "";
    for(int i = 1; i< argc; i++)
    {
        std::string arg = argv[i];
        if(arg == "-h" or arg == "--help")
        {
            printHelp();
            return 0;
        }

        if(arg == "--basePort")
        {
            i++;
            if(i<argc)
            {
                basePort = argv[i];
            }
            continue;
        }

        if(arg == "--certBasePath")
        {
            i++;
            if(i<argc)
            {
                certBasePath = argv[i];
            }
            continue;
        }

    }

    int insecurePort = std::stoi(basePort) + 51;
    int dfltPort = std::stoi(basePort) + 52;
    int securePort = std::stoi(basePort) + 443;
    grpc::ServerBuilder builder;
    if(certBasePath != "")
    {
        // Only initialize ssl ports when certBasePath is given

        // Create a default SSL Credentials object.
        std::string serverKeyPath = certBasePath + "/server_key.pem";
        std::string serverCertPath = certBasePath + "/server_crt.pem";
        std::string clientCertPath = certBasePath + "/client_crt.pem";

        std::shared_ptr<grpc::ServerCredentials> sslCreds;
        std::shared_ptr<grpc::ServerCredentials> defaultCreds;

        // Get Credentials from Files and define them as  Server Key Cert Pair (needed to fill pem_key_cert_pairs vector of SslCredentialOptions)
        // Here we can change to chain of trust instead of selfsigned
        std::string serverKey = gwhisper::util::readFromFile(serverKeyPath);
        std::cout << "Reading server key from " << serverKeyPath << std::endl;
        if (serverKey == "FAIL")
        {
            std::cout << "No server key found at " << serverKeyPath << std::endl;
            std::cout << "Stopping server" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string serverCert = gwhisper::util::readFromFile(serverCertPath);
        std::cout << "Reading server cert from " << serverCertPath << std::endl;
        if (serverCert == "FAIL")
        {
            std::cout << "No server cert found at " << serverCertPath << std::endl;
            std::cout << "Stopping server" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string clientCert = gwhisper::util::readFromFile(clientCertPath);
        std::cout << "Reading client cert from " << clientCertPath << std::endl;
        if (clientCert == "FAIL")
        {
            std::cout << "No client cert found at " << clientCertPath << std::endl;
            std::cout << "Stopping server" << std::endl;
            exit(EXIT_FAILURE);
        }

        grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {serverKey.c_str(), serverCert.c_str()};

        // Security Options for ssl connection
        grpc::SslServerCredentialsOptions sslOpts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);
        grpc::SslServerCredentialsOptions defaultOpts(GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);

        // Set credentials
        sslOpts.pem_root_certs = clientCert;
        sslOpts.pem_key_cert_pairs.push_back(pkcp);

        defaultOpts.pem_root_certs = clientCert;
        defaultOpts.pem_key_cert_pairs.push_back(pkcp);

        sslCreds = grpc::SslServerCredentials(sslOpts);
        defaultCreds = grpc::SslServerCredentials(defaultOpts);
        //grpc::ServerBuilder secureBuilder;
        //grpc::ServerBuilder insecureBuilder;


        // Default port: 50052, Server does not require key-cert-pair
        std::string dfltServerAddr = "localhost:" + std::to_string(dfltPort);
        std::cout << "Starting secure server without required client cert listening on " << dfltServerAddr << std::endl;
        builder.AddListeningPort(dfltServerAddr, defaultCreds);
        std::cout << "DEFAULT CREATED " << dfltServerAddr << std::endl;

        // SSL Port: 50443, server requires all credentials
        std::string secureServerAddr = "localhost:" + std::to_string(securePort);
        std::cout << "Starting secure server with required valid client cert-key pair listening on " << secureServerAddr << std::endl;
        builder.AddListeningPort(secureServerAddr, sslCreds);
        std::cout << "SECURE CREATED " << secureServerAddr << std::endl;
    }

    //Insecure Port: 50051. no credentials required
    std::string insecureServerAddr = "localhost:" + std::to_string(insecurePort);
    std::cout << "Starting insecure server listening on " << insecureServerAddr << std::endl;
    builder.AddListeningPort(insecureServerAddr, grpc::InsecureServerCredentials());
    std::cout << "INSECURE CREATED " << insecureServerAddr << std::endl;

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

    std::cout << "Services Registered" << std::endl;
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
    std::cout << "Soon to exit start server script" << std::endl;
    return 0;
}
