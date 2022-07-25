
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

#include "libCli/ConnectionManager.hpp"
#include <gRPC_utils/proto_reflection_descriptor_database.h>

#include "utils/gwhisperUtils.hpp"
#include "libLocalDescriptorCache/DescDbProxy.hpp"

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <libArgParse/ArgParse.hpp>
#include "GWhisperConfig.hpp"

namespace cli
{
    ConnectionManager &ConnectionManager::getInstance()
    {
        static ConnectionManager connectionManager;
        return connectionManager;
    }

    std::shared_ptr<grpc::Channel> ConnectionManager::getChannel(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
    {
        if (!findChannelByAddress(f_serverAddress))
        {
            registerConnection(f_serverAddress, f_parseTree);
        }
        return m_connections[f_serverAddress].channel;
    }

    std::shared_ptr<DescDbProxy> ConnectionManager::getDescDb(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
    {
        if (!findDescDbByAddress(f_serverAddress))
        {
            if (m_connections[f_serverAddress].channel)
            {
                ensureDescDbProxyAndDescPoolIsAvailable(f_serverAddress,f_parseTree);
            }
            else
            {
                registerConnection(f_serverAddress, f_parseTree);
            }
        }
        return m_connections[f_serverAddress].descDbProxy;
    }
    
    std::shared_ptr<grpc::protobuf::DescriptorPool> ConnectionManager::getDescPool(std::string f_serverAddress, ArgParse::ParsedElement &f_parseTree)
    {
        if (!findDescPoolByAddress(f_serverAddress))
        {
            ensureDescDbProxyAndDescPoolIsAvailable(f_serverAddress, f_parseTree);
        }
        return m_connections[f_serverAddress].descPool;
    }

    void ConnectionManager::ensureDescDbProxyAndDescPoolIsAvailable(std::string &f_serverAddress, ArgParse::ParsedElement &f_parseTree)
    {
        if (m_connections[f_serverAddress].channel)
        {
            bool disableCache = (f_parseTree.findFirstChild("DisableCache") != "");     
            m_connections[f_serverAddress].descDbProxy = std::make_shared<DescDbProxy>(disableCache, f_serverAddress, m_connections[f_serverAddress].channel, f_parseTree);
            m_connections[f_serverAddress].descPool = std::make_shared<grpc::protobuf::DescriptorPool>(m_connections[f_serverAddress].descDbProxy.get()); //Pointer in DB
        }
        else
        {
            registerConnection(f_serverAddress, f_parseTree);
        }
    }
    
    bool ConnectionManager::findChannelByAddress(std::string &f_serverAddress)
    {
        if (m_connections.find(f_serverAddress) != m_connections.end())
        {
            if (m_connections[f_serverAddress].channel != nullptr)
            {
                return true;
            }
        }
        return false;
    }

    // Check if the cached map contains the gRpc DescriptorDatabase of given the server address or not.
    bool ConnectionManager::findDescDbByAddress(std::string &f_serverAddress)
    {
        if (m_connections.find(f_serverAddress) != m_connections.end())
        {
            if (m_connections[f_serverAddress].descDbProxy != nullptr)
            {
                return true;
            }
        }
        return false;
    }
    // Check if the cached map contains the gRpc DescriptorPool of the given server address or not.
    bool ConnectionManager::findDescPoolByAddress(std::string &f_serverAddress)
    {
        if (m_connections.find(f_serverAddress) != m_connections.end())
        {
            if (m_connections[f_serverAddress].descPool != nullptr)
            {
                return true;
            }
        }
        return false;
    }

  
    void ConnectionManager::registerConnection(std::string &f_serverAddress, ArgParse::ParsedElement &f_parseTree)
    {
        ConnList connection;
        std::shared_ptr<grpc::ChannelCredentials> creds;
        std::shared_ptr<grpc::ChannelCredentials> channelCreds;
        gWhisperConfig tree2(f_parseTree); // Todo: Replacef_parseTree with config object in signature of Connectionmanager

       if (tree2.lookUpSetting("Ssl", f_parseTree) != "")
        //if (f_parseTree.findFirstChild("Ssl") != "")
        {
            // if --ssl set is set, check if user provides keys/ certs
           /* bool clientCertOption = (f_parseTree.findFirstChild("OptionClientCert") != "");
            bool clientKeyOption = (f_parseTree.findFirstChild("OptionClientKey") != "");
            bool serverCertOption = (f_parseTree.findFirstChild("OptionServerCert") != "");*/

            std::string sslClientCertPath = tree2.lookUpSetting("ClientCertFile", f_parseTree);
            std::string sslClientKeyPath = tree2.lookUpSetting("ClientKeyFile", f_parseTree);
            std::string sslServerCertPath = tree2.lookUpSetting("ServerCertFile", f_parseTree);

            // debugString = "CREATE SECURE CAHNNEL WITH USER-PROVIDED CREDENTIALS";
            channelCreds = generateSSLCredentials(sslClientCertPath, sslClientKeyPath, sslServerCertPath);
            connection.channel = grpc::CreateChannel(f_serverAddress, channelCreds);
        }
        else
        {
            // create insecure channel by default
            connection.channel = grpc::CreateChannel(f_serverAddress, grpc::InsecureChannelCredentials());
        }

        bool disableCache = (f_parseTree.findFirstChild("DisableCache") != "");
        // Timeout as chrono duration

        connection.descDbProxy = std::make_shared<DescDbProxy>(disableCache, f_serverAddress, connection.channel, f_parseTree);
        connection.descPool = std::make_shared<grpc::protobuf::DescriptorPool>(connection.descDbProxy.get());

        m_connections[f_serverAddress] = connection;
    }

    std::shared_ptr<grpc::ChannelCredentials> ConnectionManager::generateSSLCredentials(const std::string f_sslClientCertPath, const std::string f_sslClientKeyPath, const std::string f_sslServerCertPath)
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
}
