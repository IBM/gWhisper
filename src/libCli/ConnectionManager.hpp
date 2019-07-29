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

#include <third_party/gRPC_utils/proto_reflection_descriptor_database.h>

namespace cli
{
    /// List of connection infomation
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

            bool findChannelByAddress(std::string f_address)
            {
                if(connections.find(f_address) != connections.end())
                {
                    if(connections[f_address].channel != nullptr)
                    {
                        return true;
                    }
                }
                return false;
            }
            bool findDescDbByAddress(std::string f_address)
            {
                if(connections.find(f_address) != connections.end())
                {
                    if(connections[f_address].descDb != nullptr)
                    {
                        return true;
                    }
                }
                return false;
            }
            bool findDescPoolByAddress(std::string f_address)
            {
                if(connections.find(f_address) != connections.end())
                {
                    if(connections[f_address].descPool != nullptr)
                    {
                        return true;
                    }
                }
                return false;
            }

            // create a channel according to the server address if the chache map doesn't contain it
            std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, std::string f_serverPort)
            {
                if(f_serverPort == "")
                {
                    f_serverPort = "50051";
                }
                std::string serverAddress = f_serverAddress + ":" + f_serverPort;

                if(!findChannelByAddress(serverAddress))
                {
                    registerConnection(serverAddress);
                }
                return connections[serverAddress].channel;
            }

            // re-use the Descriptor Database for the corresponding channel
            std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> getDescDb(std::string f_serverAddress, std::string f_serverPort)
            {
                if(f_serverPort == "")
                {
                    f_serverPort = "50051";
                }
                std::string serverAddress = f_serverAddress + ":" + f_serverPort;

                if(!findDescDbByAddress(serverAddress))
                {
                    if(connections[serverAddress].channel)
                    {
                        connections[serverAddress].descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connections[serverAddress].channel);
                        connections[serverAddress].descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connections[serverAddress].descDb.get());
                    }
                    else
                    {
                        registerConnection(serverAddress);
                    }
                }
                return connections[serverAddress].descDb;
            }

            std::shared_ptr<grpc::protobuf::DescriptorPool> getDescPool(std::string f_serverAddress, std::string f_serverPort)
            {
                if(f_serverPort == "")
                {
                    f_serverPort = "50051";
                }
                std::string serverAddress = f_serverAddress + ":" + f_serverPort;
                if(!findDescPoolByAddress(serverAddress))
                {
                    if(connections[serverAddress].channel)
                    {
                        connections[serverAddress].descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connections[serverAddress].channel);
                        connections[serverAddress].descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connections[serverAddress].descDb.get());
                    }
                    else
                    {
                        registerConnection(serverAddress);
                    }
                }
                return connections[serverAddress].descPool;
            }
        private:
            // connection cache for resuing the channel, corresponding descriptor Database and DatabasePool
            std::unordered_map<std::string, ConnList> connections;
            void registerConnection(std::string f_serverAddress)
            {
                ConnList connection;
                connection.channel = grpc::CreateChannel(f_serverAddress, grpc::InsecureChannelCredentials());
                connection.descDb = std::make_shared<grpc::ProtoReflectionDescriptorDatabase>(connection.channel);
                connection.descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connection.descDb.get());

                if(connections.find(f_serverAddress) == connections.end())
                {
                    connections.insert(std::make_pair(f_serverAddress, connection));
                }
                else
                {
                    connections[f_serverAddress] = connection;
                }
            }
    };
}
