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
            ConnectionManager(const ConnectionManager & ) = delete;
            ConnectionManager& operator=(const ConnectionManager & ) = delete;
        private:
            ConnectionManager(){}
            ~ConnectionManager(){}

        public:
            /// Only use a single connection instance
            static ConnectionManager & getInstance()
            {
                static ConnectionManager connectionManager;
                return connectionManager;
            }

            /// To get the channel according to the server address. If the cached map doesn't contain the channel, create the connection list and update the map.
            /// @param f_serverAddress Service Addresses with Port, described in gRPC string format "hostname:port".
            /// @returns the channel of the corresponding server address.
            std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress)
            {
                if(!findChannelByAddress(f_serverAddress))
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
                if(!findDescDbByAddress(f_serverAddress))
                {
                    if(connections[f_serverAddress].channel)
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

                if(!findDescPoolByAddress(f_serverAddress))
                {
                    if(connections[f_serverAddress].channel)
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
                if(connections.find(f_serverAddress) != connections.end())
                {
                    if(connections[f_serverAddress].channel != nullptr)
                    {
                        return true;
                    }
                }
                return false;
            }
            // Check if the cached map contains the gRpc DescriptorDatabase of given the server address or not.
            bool findDescDbByAddress(std::string f_serverAddress)
            {
                if(connections.find(f_serverAddress) != connections.end())
                {
                    if(connections[f_serverAddress].descDb != nullptr)
                    {
                        return true;
                    }
                }
                return false;
            }
            // Check if the cached map contains the gRpc DescriptorPool of the given server address or not.
            bool findDescPoolByAddress(std::string f_serverAddress)
            {
                if(connections.find(f_serverAddress) != connections.end())
                {
                    if(connections[f_serverAddress].descPool != nullptr)
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
                ConnList connection;
                connection.channel = grpc::CreateChannel(f_serverAddress, grpc::InsecureChannelCredentials());
                connection.descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connection.channel);
                connection.descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connection.descDb.get());
                connections[f_serverAddress] = connection;
            }
    };
}
