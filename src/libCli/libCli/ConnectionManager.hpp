
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
#include "libLocalDescriptorCache/DescDbProxy.hpp"

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <libArgParse/ArgParse.hpp>

class DescDbProxy; // Forward Declaration to avoid cirular dependencies.
namespace cli
{
    /// Class to manage and reuse connection information, singleton pattern.
    class ConnectionManager
    {
    public:
        ConnectionManager(const ConnectionManager &) = delete;
        ConnectionManager &operator=(const ConnectionManager &) = delete;

         /// Only use a single connection instance
        static ConnectionManager &getInstance();

        /// To get the channel according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the channel of the corresponding server address.
        std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree);

        /// To get the gRpc DescriptorDatabase according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorDatabase of the corresponding server address.
        std::shared_ptr<DescDbProxy> getDescDb(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree);

        /// To get the gRpc DescriptorPool according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
        /// @returns the gRpc DescriptorPool of the corresponding server address.
        std::shared_ptr<grpc::protobuf::DescriptorPool> getDescPool(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree);

        /// @brief closes the DescDb stream with a given deadline.
        /// @param f_serverAddress server addresss to lookup the assigned DescDbProxy.
        /// @param deadline optional dealine for closing the stream.
        /// @return returns grpc::StatusCode::ABORTED status if no DescDb proxy is attached to the server address, 
        /// otherwise grpc status as a result of stream closure.
        grpc::Status closeDescDbWithDeadline(std::string f_serverAddress,
                                            std::optional<std::chrono::time_point<std::chrono::system_clock>> deadline);

    private:
        ConnectionManager() {}
        ~ConnectionManager() {}

        void ensureDescDbProxyAndDescPoolIsAvailable(std::string &f_serverAddress, ArgParse::ParsedElement &f_parseTree); 

        /// Check if the cached map contains the channel of the given server address or not.
        bool findChannelByAddress(std::string &f_serverAddress);

        /// Check if the cached map contains the gRpc DescriptorDatabase of given the server address or not.
        bool findDescDbByAddress(std::string &f_serverAddress);

        /// Check if the cached map contains the gRpc DescriptorPool of the given server address or not.
        bool findDescPoolByAddress(std::string &f_serverAddress);

        /// To register the gRpc connection information of a given server address.
        /// Connection List contains: Channel, DescriptorDatabase and DescriptorPool as value.
        /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port" as key of the cached map.
        /// @param f_parseTree Tree of the recent gWhisper command. Used to lookup which connection type should be opened (SSL or not).
        void registerConnection(std::string &f_serverAddress, ArgParse::ParsedElement &f_parseTree);

        /// @param f_sslClientCertPath Path to client certificate. Provide certificate as .pem or .crt
        /// @param f_sslClientKeyPath Path to client private key. Provide key as .pem or .key
        /// @param f_sslServerCretPath Path to server certificate. Provide Certificate as .pem or .crt
        /// @return std::shared_ptr<grpc::ChannelCredentials> gRPC credentials as used for creating an SSL/TLS channel
        std::shared_ptr<grpc::ChannelCredentials> generateSSLCredentials(const std::string f_sslClientCertPath, 
                                                const std::string f_sslClientKeyPath, const std::string f_sslServerCertPath);

        /// List of gRpc connection infomation
        typedef struct ConnList
        {
            std::shared_ptr<grpc::Channel> channel = nullptr;
            std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> descDb = nullptr;
            std::shared_ptr<DescDbProxy> descDbProxy = nullptr;
            //DescDbProxy descDbProxy;
            std::shared_ptr<grpc::protobuf::DescriptorPool> descPool = nullptr;

        } ConnList;

        /// Cached map of the gRpc connection information for resuing the channel, descriptor Database and DatabasePool
        std::unordered_map<std::string, ConnList> m_connections; //needed typedef for this.
      
    };
}
