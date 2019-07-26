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
    /// Only us a single channel instances
    class ConnectionManager
    {
        public:
            ConnectionManager(const ConnectionManager & ) = delete;
            ConnectionManager& operator=(const ConnectionManager & ) = delete;
        private:
            ConnectionManager(){};
            ~ConnectionManager(){};
        public:
			static ConnectionManager & getInstance(){

                static ConnectionManager connectionManager;
				return connectionManager;
			}

            std::shared_ptr<grpc::Channel> findChannelByAddress(std::string f_address)
			{
                return channels[f_address];
			}

            std::shared_ptr<grpc::Channel> getChannel(std::string f_serverAddress, std::string f_serverPort)
            {

                if(f_serverPort == "")
                {
                    f_serverPort = "50051";
                }
				std::string serverAddress = f_serverAddress + ":" + f_serverPort;

                std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(serverAddress, grpc::InsecureChannelCredentials());

                registerChannel(serverAddress, channel);

                return findChannelByAddress(serverAddress);
            };

            grpc::ProtoReflectionDescriptorDatabase & getDescDb(std::shared_ptr<grpc::Channel> f_channel)
            {
                static grpc::ProtoReflectionDescriptorDatabase descDb(f_channel);
                return descDb;
            }

            grpc::protobuf::DescriptorPool & getDescPool(std::shared_ptr<grpc::Channel> f_channel)
            {
                grpc::ProtoReflectionDescriptorDatabase & descDb = ConnectionManager::getDescDb(f_channel);
                static grpc::protobuf::DescriptorPool descPool(&descDb);
                return descPool;
            }
        private:
            std::unordered_map<std::string, std::shared_ptr<grpc::Channel>> channels;
            void registerChannel(std::string f_serverAddress, std::shared_ptr<grpc::Channel> f_channel)
            {
                if(channels.find(f_serverAddress) == channels.end())
                {
                    channels.insert(std::make_pair(f_serverAddress, f_channel));
                }
            };
    };
}