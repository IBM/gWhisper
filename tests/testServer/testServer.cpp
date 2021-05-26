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
#include <fstream>

#include <grpcpp/grpcpp.h>
#include "ServiceScalarTypeRpcs.hpp"
#include "ServiceStreamingRpcs.hpp"
#include "ServiceComplexTypeRpcs.hpp"
#include "ServiceNestedTypeRpcs.hpp"
#include "ServiceStatusHandling.hpp"
#include <grpcpp/security/credentials.h>
/// Function for reading and returning credentials (Key, Cert) for secure server
std::string readFromFile(const char f_path[])
{
    std::ifstream credFile(f_path);
    //credFile.open(f_path);
    //d::string fileContent;
    if (credFile)
    {
        char nextChar;

        std::string str{std::istreambuf_iterator<char>(credFile),
                        std::istreambuf_iterator<char>()};

        if (credFile)
            std::cout << "Server: File content of " << f_path << ": " << str << std::endl;

        return str;
    }
    else
    {
        std::cout << "No cert/key found at " << f_path << std::endl;
        std::cout << "Stopping server" << std::endl;
        exit(EXIT_FAILURE);
    }
}

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
    //std::string serverAddr = "0.0.0.0:50051";
    std::string secureServerAddr = "localhost:50443";
    std::string scrChnlServerAddr = "localhost:50052";
    std::string serverAddr = "localhost:50051";
    if (argc >= 2)
    {
        //serverAddr = "0.0.0.0:" + std::string(argv[1]);
        serverAddr = "localhost:" + std::string(argv[1]);
        std::cout << argv[1] << std::endl;
    }

    //std::cout << argv[1] << std::endl;

    // Frage: Wo wird der Channel geöffnet? Hier create ich doch nur einen Insecure Server
    // Wie enable ich TLS für den Server? Ich habe da keinen Parameter gefunden. Genügt es über den TLS Port zu gehen?:
    // std::shared_ptr< grpc::SslServerCredentials > secureCredentials
    // builder.AddListeningPort(addr+":465", creds )

    // std::cout << "Starting server listening on " << serverAddr << std::endl;
    // grpc::ServerBuilder builder;
    // builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());

    if (argv[1] == "-ssl")
    {
        std::cout << "Flag is working" << std::endl;
    }
    else
    {
        std::cout << "Flag is not working" << std::endl;

        std::cout << serverAddr << std::endl;
    }
    std::cout << "Starting secure server listening on " << secureServerAddr << std::endl;
    std::cout << "Starting server for secur channel listening on " << scrChnlServerAddr << std::endl;
    std::cout << "Starting insecure server listening on " << serverAddr << std::endl;
    // Create a default SSL Credentials object.
    //const char *serverKeyPath = "../cert-key-pairs/serverPrivateKey.key";
    //const char *serverCertPath = "../cert-key-pairs/serverCert.crt";
    //const char *clientCertPath = "../cert-key-pairs/clientCert.crt";

    const char serverKeyPath[] = "../cert-key-pairs/server_key.pem";
    const char serverCertPath[] = "../cert-key-pairs/server_crt.pem";
    const char clientCertPath[] = "../cert-key-pairs/client_crt.pem";

    std::shared_ptr<grpc::ServerCredentials> sslCreds;
    std::shared_ptr<grpc::ServerCredentials> defaultCreds;

    // Get Credentials from Files and define them as  Server Key Cert Pair (needed to fill pem_key_cert_pairs vector of SslCredentialOptions)
    // Here we can change to chain of trust instead of selfsigned
    std::string serverKey = readFromFile(serverKeyPath);
    std::string serverCert = readFromFile(serverCertPath);
    std::string clientCert = readFromFile(clientCertPath);

    grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {serverKey.c_str(), serverCert.c_str()};

    // Security Options for ssl connection
    grpc::SslServerCredentialsOptions sslOpts(GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY);
    grpc::SslServerCredentialsOptions defaultOpts(GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);

    // Set credentials
    sslOpts.pem_root_certs = clientCert;
    //sslOpts.pem_root_certs = "";
    sslOpts.pem_key_cert_pairs.push_back(pkcp);

    defaultOpts.pem_root_certs = clientCert;
    defaultOpts.pem_key_cert_pairs.push_back(pkcp);

    sslCreds = grpc::SslServerCredentials(sslOpts);
    defaultCreds = grpc::SslServerCredentials(defaultOpts);
    grpc::ServerBuilder builder;
    grpc::ServerBuilder secureBuilder;
    grpc::ServerBuilder insecureBuilder;

    // Default port
    builder.AddListeningPort(scrChnlServerAddr, defaultCreds);
    std::cout << "DEFAULT CREATED " << scrChnlServerAddr << std::endl;
    // SSL Port
    secureBuilder.AddListeningPort(secureServerAddr, sslCreds);
    std::cout << " SECURE CREATED " << secureServerAddr << std::endl;
    //Insecure Port
    insecureBuilder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
    std::cout << "INSECURE CREATED " << serverAddr << std::endl;

    // register all services:
    ServiceScalarTypeRpcs scalarTypeRpcs;
    builder.RegisterService(&scalarTypeRpcs);
    secureBuilder.RegisterService(&scalarTypeRpcs);
    insecureBuilder.RegisterService(&scalarTypeRpcs);

    ServiceNestedTypeRpcs nestedTypeRpcs;
    builder.RegisterService(&nestedTypeRpcs);
    secureBuilder.RegisterService(&nestedTypeRpcs);
    insecureBuilder.RegisterService(&nestedTypeRpcs);

    ServiceComplexTypeRpcs complexTypeRpcs;
    builder.RegisterService(&complexTypeRpcs);
    secureBuilder.RegisterService(&complexTypeRpcs);
    insecureBuilder.RegisterService(&complexTypeRpcs);

    ServiceStreamingRpcs streamingRpcs;
    builder.RegisterService(&streamingRpcs);
    secureBuilder.RegisterService(&streamingRpcs);
    insecureBuilder.RegisterService(&streamingRpcs);

    ServiceStatusHandling statusHandling;
    builder.RegisterService(&statusHandling);
    secureBuilder.RegisterService(&statusHandling);
    insecureBuilder.RegisterService(&statusHandling);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::unique_ptr<grpc::Server> secureServer(secureBuilder.BuildAndStart());
    std::unique_ptr<grpc::Server> insecureServer(insecureBuilder.BuildAndStart());
    if (server != nullptr || secureServer != nullptr || insecureServer != nullptr)
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
