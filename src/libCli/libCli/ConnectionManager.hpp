
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

#include <gRPC_utils/proto_reflection_descriptor_database.h>

#include "utils/gwhisperUtils.hpp"

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <libArgParse/ArgParse.hpp>

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

    public:
        /// Only use a single connection instance
        static ConnectionManager &getInstance()
        {
            static ConnectionManager connectionManager;
            return connectionManager;
        }

        /// To get the channel according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the channel of the corresponding server address.
        std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
        {
            if (!findChannelByAddress(f_serverAddress))
            {
                registerConnection(f_serverAddress, f_parseTree);
            }
            return connections[f_serverAddress].channel;
        }

        /// To get the gRpc DescriptorDatabase according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorDatabase of the corresponding server address.
        std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> getDescDb(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
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
                    registerConnection(f_serverAddress, f_parseTree);
                }
            }
            return connections[f_serverAddress].descDb;
        }
        /// To get the gRpc DescriptorPool according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorPool of the corresponding server address.
        std::shared_ptr<grpc::protobuf::DescriptorPool> getDescPool(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
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
                    registerConnection(f_serverAddress, f_parseTree);
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
        /// @param f_parseTree Tree of the recent gWhisper command. Used to lookup which connection type should be opened (SSL or not).
        void registerConnection(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
        {
            ConnList connection;
            std::shared_ptr<grpc::ChannelCredentials> creds;
            std::shared_ptr<grpc::ChannelCredentials> channelCreds;

            if (f_parseTree.findFirstChild("ssl") != "")
            {
                // if --ssl set is set, check if user provides keys/ certs
                bool clientCertOption = (f_parseTree.findFirstChild("OptionClientCert") != "");
                bool clientKeyOption = (f_parseTree.findFirstChild("OptionClientKey") != "");
                bool serverCertOption = (f_parseTree.findFirstChild("OptionServerCert") != "");

                std::string sslClientCertPath = f_parseTree.findFirstChild("FileClientCert");
                std::string sslClientKeyPath = f_parseTree.findFirstChild("FileClientKey");
                std::string sslServerCertPath = f_parseTree.findFirstChild("FileServerCert");

                // debugString = "CREATE SECURE CAHNNEL WITH USER-PROVIDED CREDENTIALS";
                channelCreds = generateSSLCredentials(sslClientCertPath, sslClientKeyPath, sslServerCertPath);
                connection.channel = grpc::CreateChannel(f_serverAddress, channelCreds);
            }
            else
            {
                // create insecure channel by default
                connection.channel = grpc::CreateChannel(f_serverAddress, grpc::InsecureChannelCredentials());
            }

            //std::cout << "Created Channel: " << connection.channel << std::endl;
            connection.descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connection.channel);
            connection.descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connection.descDb.get());
            connections[f_serverAddress] = connection;
        }

        /// @param f_sslClientCertPath Path to client certificate. Provide certificate as .pem or .crt
        /// @param f_sslClientKeyPath Path to client private key. Provide key as .pem or .key
        /// @param f_sslServerCretPath Path to server certificate. Provide Certificate as .pem or .crt
        /// @return std::shared_ptr<grpc::ChannelCredentials> gRPC credentials as used for creating an SSL/TLS channel
        std::shared_ptr<grpc::ChannelCredentials> generateSSLCredentials(const std::string f_sslClientCertPath, const std::string f_sslClientKeyPath, const std::string f_sslServerCertPath)
        {

            std::string clientCert = "";
            std::string clientKey = "";
            std::string serverCert = "";

            grpc::SslCredentialsOptions sslOpts;

            // Read credentials from files. If file is not found / cannot be opened terminate
            // If no client key / cert were specified and a valid client key-cert pair is not required, continue without these files.
            if (f_sslClientKeyPath != "")
            {
                clientKey = gwhisper::util::readFromFile(f_sslClientKeyPath);

                if (clientKey == "FAIL")
                {
                    std::cerr << "Error while fetching clientKey from: " << f_sslClientKeyPath << std::endl;
                    std::cerr << "Failed to build secure channel" << std::endl;
                    exit(EXIT_FAILURE);
                }
                sslOpts.pem_private_key = clientKey;
            }

            if (f_sslClientCertPath != "")
            {
                clientCert = gwhisper::util::readFromFile(f_sslClientCertPath);

                if (clientCert == "FAIL")
                {
                    std::cerr << "Error while fetching clientCert from: " << f_sslClientCertPath << std::endl;
                    std::cerr << "Failed to build secure channel" << std::endl;
                    exit(EXIT_FAILURE);
                }
                sslOpts.pem_cert_chain = clientCert;
            }

            if (f_sslServerCertPath != "")
                serverCert = gwhisper::util::readFromFile(f_sslServerCertPath);
            {
                if (serverCert == "FAIL")
                {
                    std::cerr << "Error while fetching serverCert from: " << f_sslServerCertPath << std::endl;
                    std::cerr << "Failed to build secure channel" << std::endl;
                    exit(EXIT_FAILURE);
                }
                sslOpts.pem_root_certs = serverCert;
            }

            std::shared_ptr<grpc::ChannelCredentials> creds = grpc::SslCredentials(sslOpts);

            return creds;
        }
    };
}