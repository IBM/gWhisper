// Copyright 2021 IBM Corporation
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

#include "DescDbProxy.hpp"
#include "../utils/gwhisperUtils.hpp"
#include "libCli/ConnectionManager.hpp"
#include "libArgParse/ArgParse.hpp"
#include "LocalDescDb.pb.h"

#include <string>
#include <deque>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <versionDefine.h> // generated during build

#include <google/protobuf/descriptor.h>
#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
#include <google/protobuf/util/time_util.h>
#include <grpcpp/impl/codegen/config_protobuf.h>
#include <libCli/cliUtils.hpp>

bool DescDbProxy::FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output)
{
    return m_localDB.FindFileByName(filename, output);                 
}

bool DescDbProxy::FindFileContainingSymbol(const std::string& symbol_name,
                                        grpc::protobuf::FileDescriptorProto* output)
{
    return m_localDB.FindFileContainingSymbol(symbol_name, output);                 
}                              

bool DescDbProxy::FindFileContainingExtension(const std::string& containing_type,
                                        int f_field_number,
                                        grpc::protobuf::FileDescriptorProto* output)
{
    return m_localDB.FindFileContainingExtension(containing_type, f_field_number, output);                 
}       

std::vector<grpc::string> DescDbProxy::GetServices()
{
    // DescDBProxy constructor tries to fill m_service list either via reflection or via cache file
    return m_serviceList;
}  

bool DescDbProxy::isValidHostEntry(const localDescDb::DescriptorDb& f_descDb, const std::string &f_hostAddress)
{    

    for (int i=0; i < f_descDb.hosts_size(); i++)
    {
        const localDescDb::Host host = f_descDb.hosts(i);
        if (host.host_address() == f_hostAddress)
        {
            // Get current Time and Time of last Upate in Seconds
            using TimeUtil = google::protobuf::util::TimeUtil;
            google::protobuf::Timestamp currentTime = TimeUtil::GetCurrentTime(); // time in UTC
            google::protobuf::int64 secCurrentTime = TimeUtil::TimestampToSeconds(currentTime);
            
            google::protobuf::Timestamp lastUpdate = host.last_update();
            google::protobuf::int64 secLastUpdate = TimeUtil::TimestampToSeconds(lastUpdate);

            google::protobuf::int64 timeDiff = secCurrentTime - secLastUpdate;

            // Expiry for validity of an entry
            if (timeDiff <= m_cacheTimeoutInSeconds)
            {
                return true;
            }      
         return false;        
        }
    }
    return false;
}

/// Helper class to write representation of a proto file in memory to Disk.
/// @param f_protoFile Proto message that is written to disk.
/// @param f_outputFile Name of file this function writes in.
void writeProtoMsgToDisk(localDescDb::DescriptorDb& f_protoFile, std::string f_outputFilePath)
{
    std::fstream output(f_outputFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!f_protoFile.SerializeToOstream(&output)){
        std::cerr << "Failed to write "<< f_outputFilePath << std::endl;
    }
   output.close();
}

/// Remove all instances in DB of same host.
/// Writes all other hosts in new messages and returns this new message into descDb variable.
/// @param out_descDb New cache representation in memory without duplicate host entry
/// @param f_hostAddress Check, if this host is a duplicate. 
void deleteDuplicateHostEntries(localDescDb::DescriptorDb& out_descDb, const std::string f_hostAddress)
{
    localDescDb::DescriptorDb newDescDb;
    for(int i=0; i<out_descDb.hosts_size(); i++)
    {
        const localDescDb::Host& host = out_descDb.hosts(i);
        if(host.host_address() != f_hostAddress)
        {
            localDescDb::Host* newHost = newDescDb.add_hosts(); //returns host
            newHost->CopyFrom(host);
        }
    }
    out_descDb = newDescDb;
}

void DescDbProxy::repopulateLocalDb(localDescDb::Host& f_out_host, const std::string &f_hostAddress)
{
    useReflection(f_hostAddress);

    f_out_host.set_host_address(f_hostAddress);
    (*(f_out_host.mutable_last_update())) = google::protobuf::util::TimeUtil::GetCurrentTime();

    // Add service to loacal DB
    for(const auto& serviceName: (m_serviceList))
    { 
        f_out_host.add_service_list(serviceName); 
    }

    // Add all descriptors to DB entry
    for (const auto& fileName: (m_descNames))
    {
        grpc::protobuf::FileDescriptorProto  output;
        if(m_reflectionDescDb == nullptr)
        {
            std::cerr << "Error while opening DescDb: Reflection Db was not initialised." << std::endl;
            exit(EXIT_FAILURE);
        }
        if(!(m_reflectionDescDb->FindFileByName(fileName, &output))){
            std::cerr << "Could not find file " << fileName << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string dbDescEntry = output.SerializeAsString();
        f_out_host.add_file_descriptor_proto(dbDescEntry); 
    }
}

void DescDbProxy::useReflection(const std::string &f_hostAddress)
{
    if (not cli::waitForChannelConnected(m_channel, cli::getConnectTimeoutMs(&m_parseTree)))
    {
        std::cerr << "Error: Could not establish Channel. Try checking network connection, hostname or SSL credentials." << std::endl;
        exit(EXIT_FAILURE);
    }
    fetchDescNamesFromReflection(f_hostAddress);
}

void DescDbProxy::fetchDescNamesFromReflection(const std::string &f_hostAddress)
{
    if (m_channel == nullptr)
    {
        std::cerr<<"Channel is nullpointer"<<std::endl;
        exit(EXIT_FAILURE);
    }

    m_reflectionDescDb = std::make_unique<grpc::ProtoReflectionDescriptorDatabase>(m_channel);
    
    if(!(m_reflectionDescDb->GetServices(&m_serviceList)))
    {
        std::cerr << "Error while fetching services from reflection" << std::endl;
        exit(EXIT_FAILURE);      
    }

    google::protobuf::DescriptorPool descPool(m_reflectionDescDb.get());

    for(const auto& serviceName: (m_serviceList))
    {
        // Get service file names through services in descPool
        const grpc::protobuf::ServiceDescriptor * serviceDesc;
        serviceDesc = descPool.FindServiceByName(serviceName);
        if (serviceDesc == nullptr)
        {
            continue;
        }

        // Use filenames to retrieve service descriptors from ReflectionDB
        const grpc::protobuf::FileDescriptor * serviceFileDesc = serviceDesc->file();
        if (serviceFileDesc == nullptr)
        {
            std::cerr << "Service file decriptor for service " << serviceName << "is NULL" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Retrieve all proto files used by the service
        int dependencyCounter = serviceFileDesc->dependency_count();

        m_descNames.insert(serviceFileDesc->name());

        for (int i=0; i<dependencyCounter; i++){
            const grpc::protobuf::FileDescriptor * dependencyDesc = serviceFileDesc->dependency(i);
            if(dependencyDesc==nullptr)
            {
                std::cerr << "File decriptor for imported file with dependecy count " << i << " is NULL" << std::endl;
                exit(EXIT_FAILURE);
            }

            getDependencies(*dependencyDesc);
        }
    }
}

void DescDbProxy::getDependencies(const grpc::protobuf::FileDescriptor& f_parentDesc)
{       
    std::deque<const grpc::protobuf::FileDescriptor*> todoList;
    std::vector<const grpc::protobuf::FileDescriptor*> doneList;

    todoList.push_front(&f_parentDesc);

    while (not todoList.empty())
    {
        if(std::find(doneList.begin(), doneList.end(),todoList.front()) != doneList.end()){
            // Discard already processed descriptors
            todoList.pop_front();
        }
        else
        {
            int amountChildren;
            amountChildren = todoList.front()->dependency_count();
            for (int c=0; c < amountChildren; c++)
            {    
                todoList.push_back(todoList.front()->dependency(c));
            }
            std::string currentFileName = todoList.front()->name();  
            m_descNames.insert(currentFileName); // insert prevents duplicates
            doneList.push_back(todoList.front());
            todoList.pop_front();    
        }          
    }       
}

void DescDbProxy::convertHostEntryToSimpleDescDb(localDescDb::DescriptorDb f_dbProtoFile, const std::string &f_hostAddress)
{
    // Get descriptors for host entry and add them to simpleDescDb object m_localDB
    for (int i=0; i < f_dbProtoFile.hosts_size(); i++)
    {
        const localDescDb::Host host = f_dbProtoFile.hosts(i);
        if (host.host_address() == f_hostAddress)
        {
            for (int i=0; i < host.file_descriptor_proto_size(); i++)
            {
                google::protobuf::FileDescriptorProto descriptor;
                google::protobuf::FileDescriptorProto output;
                descriptor.ParseFromString(host.file_descriptor_proto(i));
                if (!m_localDB.FindFileByName(descriptor.name(), &output))
                {
                    m_localDB.Add(descriptor); // Add() prevents duplicates
                }
            }   
            break;                    
        }
    }
}

void DescDbProxy::addServicNamesToserviceList(localDescDb::DescriptorDb f_dbProtoFile, const std::string &f_hostAddress)
{
    // Add services to localDB variable if DescDb on file was valid
    std::string serviceName;

    for (int i = 0; i < f_dbProtoFile.hosts_size(); i++)
    { 
        const localDescDb::Host host = f_dbProtoFile.hosts(i); 
        if (host.host_address() == f_hostAddress)
        {
            for(int j=0; j < host.service_list_size(); j++)
            {
                serviceName = host.service_list(j); 
                (m_serviceList).push_back(serviceName);
            }
            break;
        }     
    }
}

std::string DescDbProxy::prepareCacheFile()
{
    std::string homeFolder = getenv("HOME");
    std::string cacheFolderPath = homeFolder + "/.cache/gwhisper";
    std::string dbFileName = "DescriptorCache.bin";
    std::string dbStatus = gwhisper::util::createFolder(cacheFolderPath);
    if(dbStatus == "FAIL")
    {
        std::cerr << "Failed to access folder to store cache at "<< cacheFolderPath << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string dbFilePath = cacheFolderPath + "/" + dbFileName;
    return dbFilePath;
}

void DescDbProxy::getDescriptors(const std::string &f_hostAddress)
{  
    std::string cacheFilePath = prepareCacheFile();

    // Import .prot file
    std::fstream input;
    input.open(cacheFilePath, std::ios::in | std::ios::binary);
    localDescDb::DescriptorDb dbFile;
    if(!input)
    {
        if(std::filesystem::exists(cacheFilePath)){
            std::cerr<<"Failed to open "<< cacheFilePath << ": "<<std::strerror(errno)<<std::endl;
            exit(EXIT_FAILURE);
        }
        //std::cerr << "Cannot open file, file does not exist. Creating new file.." << std::endl;
    }
    else if (!dbFile.ParseFromIstream(&input)) 
    {
      std::cerr << "Failed to parse local Descriptor Cache." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::string gwhisperBuildVersion = GWHISPER_BUILD_VERSION;
    bool accessedReflectionDb = false;

    // Add/Update DB entry for new/outdated host (via Reflection)
    if(!isValidHostEntry(dbFile, f_hostAddress) || (dbFile.gwhisper_version() != gwhisperBuildVersion))
    {
        deleteDuplicateHostEntries(dbFile, f_hostAddress);
        dbFile.clear_gwhisper_version();
        dbFile.set_gwhisper_version(gwhisperBuildVersion);
        repopulateLocalDb(*(dbFile.add_hosts()), f_hostAddress);
        accessedReflectionDb = true;
    }

    if(!accessedReflectionDb)
    {
      addServicNamesToserviceList(dbFile, f_hostAddress);
    }

    convertHostEntryToSimpleDescDb(dbFile, f_hostAddress);

    // Overwrite DB file on disc
    if(accessedReflectionDb)
    {
        writeProtoMsgToDisk(dbFile, cacheFilePath);
    }
}

grpc::Status DescDbProxy::closeDescDbStream(std::optional<std::chrono::time_point<std::chrono::system_clock>> deadline)
{
    if( m_disableCache  == false )//cache enabled, no reflection stream required.
    {
        return grpc::Status::OK;
    }

    if ( m_reflectionDescDb == nullptr )
    {
        std::cerr << "Exit - no reflectionDescDb initialized." <<std::endl;
        exit(EXIT_FAILURE);
    }
    return m_reflectionDescDb->closeStreamWithDeadline(deadline);
}

DescDbProxy::DescDbProxy(bool disableCache, const std::string &hostAddress, std::shared_ptr<grpc::Channel> channel, 
                                                                            ArgParse::ParsedElement &parseTree)
{
    m_channel = channel;
    m_parseTree = parseTree;
    m_disableCache = disableCache;
    if(disableCache)
    {
        // Get Desc directly via reflection and without touching localDB
        useReflection(hostAddress);
        for(auto &name:(m_descNames))
        {
            google::protobuf::FileDescriptorProto descriptor;
            if (m_reflectionDescDb == nullptr)
            {
                std::cerr<<"Exit in DescDb constr"<<std::endl;
                exit(EXIT_FAILURE);
            }
            m_reflectionDescDb->FindFileByName(name, &descriptor);
            m_localDB.Add(descriptor);
        }     

    }
    else
    {
        getDescriptors(hostAddress);
    }
}

DescDbProxy::~DescDbProxy(){}