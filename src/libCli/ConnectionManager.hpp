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

            // create a channel according to the server address if the chache map doesn't contain it
            std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, std::string f_serverPort)
            {
                if(!findChannelByAddress(f_serverAddress))
                {
                    registerConnection(f_serverAddress);
                }
                return connections[f_serverAddress].channel;
            }

            // re-use the Descriptor Database for the corresponding channel
            std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> getDescDb(std::string f_serverAddress, std::string f_serverPort)
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

            std::shared_ptr<grpc::protobuf::DescriptorPool> getDescPool(std::string f_serverAddress, std::string f_serverPort)
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
            // connection cache for resuing the channel, corresponding descriptor Database and DatabasePool
            std::unordered_map<std::string, ConnList> connections;
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
