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

#include <fstream>

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>

#include <third_party/gRPC_utils/proto_reflection_descriptor_database.h>

namespace cli
{
    /// List of gRpc connection infomation
    typedef struct ConnList
    {
        std::shared_ptr<grpc::Channel> channel = nullptr;
        std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> descDb = nullptr;
        std::shared_ptr<grpc::protobuf::DescriptorPool> descPool = nullptr;

    } ConnList;

    /// Class to manage and resuse connection information, singleton pattern.
    class ConnectionManager
    {
    public:
        ConnectionManager(const ConnectionManager &) = delete;
        ConnectionManager &operator=(const ConnectionManager &) = delete;

    private:
        ConnectionManager() {}
        ~ConnectionManager() {}
        int connectionStatus;

    public:
        /// Only use a single connection instance
        static ConnectionManager &getInstance()
        {
            static ConnectionManager connectionManager;
            return connectionManager;
        }

        /// Get CommandlineArgs from Main. Needed for Deciding which Channel to build

        void setConnectionStatus(const int f_connectionStatus)
        // Is static here ok? For me Connection status is const, that is why i used static here. If static is not ok, gereate connectionmanager-object in gwhisper.cpp
        {
            if ((f_connectionStatus == 0) || (f_connectionStatus == 1) || (f_connectionStatus == 2))
            {
                connectionStatus = f_connectionStatus;
                std::cout << "Connection Status: " << f_connectionStatus << std::endl;
            }
            else
            {
                std::cerr << "Could not set Connection Status" << std::endl;
            }
        }

        int getConnectionStatus()
        {
            //not sure if getter is needed.
            return connectionStatus;
        }

        /// To get the channel according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the channel of the corresponding server address.
        std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress)
        {
            if (!findChannelByAddress(f_serverAddress))
            {
                registerConnection(f_serverAddress);
            }
            return connections[f_serverAddress].channel;
        }

        /// To get the gRpc DescriptorDatabase according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorDatabase of the corresponding server address.
        std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> getDescDb(std::string f_serverAddress)
        {
            if (!findDescDbByAddress(f_serverAddress))
            {
                if (connections[f_serverAddress].channel)
                {
                    connections[f_serverAddress].descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connections[f_serverAddress].channel);
                    connections[f_serverAddress].descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connections[f_serverAddress].descDb.get());
                }
                else
                {
                    registerConnection(f_serverAddress);
                }
            }
            return connections[f_serverAddress].descDb;
        }
        /// To get the gRpc DescriptorPool according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorPool of the corresponding server address.
        std::shared_ptr<grpc::protobuf::DescriptorPool> getDescPool(std::string f_serverAddress)
        {

            if (!findDescPoolByAddress(f_serverAddress))
            {
                if (connections[f_serverAddress].channel)
                {
                    connections[f_serverAddress].descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connections[f_serverAddress].channel);
                    connections[f_serverAddress].descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connections[f_serverAddress].descDb.get());
                }
                else
                {
                    registerConnection(f_serverAddress);
                }
            }
            return connections[f_serverAddress].descPool;
        }

    private:
        // Cached map of the gRpc connection information for resuing the channel, descriptor Database and DatabasePool
        std::unordered_map<std::string, ConnList> connections;
        // Check if the cached map contains the channel of the given server address or not.
        bool findChannelByAddress(std::string f_serverAddress)
        {
            if (connections.find(f_serverAddress) != connections.end())
            {
                if (connections[f_serverAddress].channel != nullptr)
                {
                    return true;
                }
            }
            return false;
        }
        // Check if the cached map contains the gRpc DescriptorDatabase of given the server address or not.
        bool findDescDbByAddress(std::string f_serverAddress)
        {
            if (connections.find(f_serverAddress) != connections.end())
            {
                if (connections[f_serverAddress].descDb != nullptr)
                {
                    return true;
                }
            }
            return false;
        }
        // Check if the cached map contains the gRpc DescriptorPool of the given server address or not.
        bool findDescPoolByAddress(std::string f_serverAddress)
        {
            if (connections.find(f_serverAddress) != connections.end())
            {
                if (connections[f_serverAddress].descPool != nullptr)
                {
                    return true;
                }
            }
            return false;
        }
        /// To register the gRpc connection information of a given server address.
        /// Connection List contains: Channel, DescriptorDatabase and DescriptorPool as value.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port" as key of the cached map.
        void registerConnection(std::string f_serverAddress)
        {

            //TODO: Create one Default Channel (Default Certs, channel ssh), no further flags/ options needed
            // Create 2 other Channels based on user Input:
            // 1 Secure Channel: connects to TLS Port of Test Server. Needs --ssl Flag as well as Path  to User defined Credentials
            // --> Do this maybe as CL
            // 1 Insecure Channel: connects to 50051 Port of Test server --noSsl Flag needed,

            ConnList connection;
            std::shared_ptr<grpc::ChannelCredentials> creds;
            int connectionStatus = getConnectionStatus();

            const char clientKeyPath[] = "../cert-key-pairs/client_key.pem";
            const char clientCertPath[] = "../cert-key-pairs/client_crt.pem";
            const char serverCertPath[] = "../cert-key-pairs/server_crt.pem";

            switch (connectionStatus)
            {
            case 0:
                //create insecure channel
                std::cout << "CREATE INSECURE CAHNNEL" << std::endl;
                break;
            case 1:
                //create secure channel
                std::cout << "CREATE SECURE CAHNNEL" << std::endl;
                break;
            default:
                std::cout << "CREATE DEFAULT CAHNNEL" << std::endl;
                //create secure channel
            }

            std::shared_ptr<grpc::ChannelCredentials> channelCreds = getCredentials(clientCertPath, clientKeyPath, serverCertPath);
            if (!channelCreds)
            {
                std::cout << "No / Wrong Channel Credentials" << std::endl;
            }
            else
            {
                std::cout << "*****************************************************************************************************" << std::endl;
                std::cout << "Channel Credentials: " << channelCreds << std::endl;
            }

            std::cout << "*****************************************************************************************************" << std::endl;
            std::cout << "Conntecting to Server: " << f_serverAddress << std::endl;
            connection.channel = grpc::CreateChannel(f_serverAddress, channelCreds);

            std::cout << "*****************************************************************************************************" << std::endl;
            std::cout << "Created Channel: " << connection.channel << std::endl;
            connection.descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connection.channel);
            connection.descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connection.descDb.get());
            connections[f_serverAddress] = connection;
        }
        /// Get Key-Cert Pairs from Files and use them as credentials for secure Channel
        std::shared_ptr<grpc::ChannelCredentials> getCredentials(const char f_clientCertPath[], const char f_clientKeyPath[], const char f_serverCertPath[])
        {

            std::cout << "*****************************************************************************************************" << std::endl;
            std::cout << "ClientCertPath: " << f_clientCertPath << std::endl;
            std::cout << "ClientKeyPath: " << f_clientKeyPath << std::endl;
            std::cout << "ServerCertPath: " << f_serverCertPath << std::endl;

            std::string clientKey = readFromFile(f_clientKeyPath);
            std::string clientCert = readFromFile(f_clientCertPath);
            std::string serverCert = readFromFile(f_serverCertPath);

            grpc::SslCredentialsOptions sslOpts;
            sslOpts.pem_private_key = clientKey;
            sslOpts.pem_cert_chain = clientCert;
            sslOpts.pem_root_certs = serverCert;

            std::shared_ptr<grpc::ChannelCredentials> creds = grpc::SslCredentials(sslOpts);
            return creds;
        }
        /// Function for reading and returning credentials (Key, Cert) for secure server
        std::string readFromFile(const char f_path[])
        {
            std::ifstream credFile(f_path);
            if (f_path)
            {

                std::string str{std::istreambuf_iterator<char>(credFile),
                                std::istreambuf_iterator<char>()};

                // std::cout << "Channel: File content of " << f_path << ": " << str << std::endl;
                return str;
            }
            else
            {
                std::cout << "No cert/key found at " << f_path << std::endl;
                std::cout << "Failed to build secure channel";
                exit(EXIT_FAILURE);
            }
        }
    };
}
