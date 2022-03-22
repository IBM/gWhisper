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
#include "LocalDescDb.pb.h"

#include<string>
#include <deque>
#include <iostream>
#include <fstream>

#include <versionDefine.h> // generated during build

#include <google/protobuf/descriptor.h>
#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
#include <google/protobuf/util/time_util.h>
#include <grpcpp/impl/codegen/config_protobuf.h>

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

bool DescDbProxy::isValidHostEntry(const localDescDb::DescriptorDb& f_descDb, const std::string f_hostAddress)
{    

    for (int i=0; i < f_descDb.hosts_size(); i++)
    {
        const localDescDb::Host host = f_descDb.hosts(i);
        if (host.hostaddress() == f_hostAddress)
        {
            // Get current Time and Time of last Upate in Seconds
            using TimeUtil = google::protobuf::util::TimeUtil;
            google::protobuf::Timestamp currentTime = TimeUtil::GetCurrentTime(); // time in UTC
            google::protobuf::int64 secCurrentTime = TimeUtil::TimestampToSeconds(currentTime);
            
            google::protobuf::Timestamp lastUpdate = host.lastupdate();
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

/// Helper class to write representation of a proto file in RAM to Disk.
/// @param f_protoFile Proto message that is written to disk.
/// @param f_outputFile Name of file this function writes in.
void writeProtoMsgToDisk(localDescDb::DescriptorDb& f_protoFile, std::string f_outputFilePath)
{
    std::fstream output(f_outputFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!f_protoFile.SerializeToOstream(&output)){
        std::cerr << "Failed to write "<< f_outputFilePath << std::endl;
    }
}

// Remove all instances in DB of same host.
// Writes all other hosts in new messages and returns this new message into descDb variable.
void deleteDuplicateHostEntries(localDescDb::DescriptorDb& out_descDb, const std::string f_hostAddress, const std::string f_dbFile)
{
    localDescDb::DescriptorDb newDescDb;
    for(int i=0; i<out_descDb.hosts_size(); i++)
    {
        const localDescDb::Host& host = out_descDb.hosts(i);
        if(host.hostaddress() != f_hostAddress)
        {
            localDescDb::Host* newHost = newDescDb.add_hosts(); //returns host
            newHost->CopyFrom(host);
        }
    }
    out_descDb = newDescDb;
}


void DescDbProxy::repopulateLocalDb(localDescDb::Host& f_out_host, const std::string &f_hostAddress)
{
    fetchDescNamesFromReflection();

    f_out_host.set_hostaddress(f_hostAddress);
    (*(f_out_host.mutable_lastupdate())) = google::protobuf::util::TimeUtil::GetCurrentTime();

    // Add service to loacal DB
    for(const auto& serviceName: (m_serviceList))
    { 
        f_out_host.add_servicelist(serviceName); 
    }

    // Add all descriptors to DB entry
    for (const auto& fileName: (m_descNames))
    {
        grpc::protobuf::FileDescriptorProto  output;

        if(!(m_reflectionDescDb.FindFileByName(fileName, &output))){
            std::cerr << "Could not find file " << fileName << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string dbDescEntry = output.SerializeAsString();
        f_out_host.add_file_descriptor_proto(dbDescEntry); 
    }
}

void DescDbProxy::fetchDescNamesFromReflection()
{
    if(!(m_reflectionDescDb.GetServices(&m_serviceList)))
    {
        std::cerr << "Error while fetching services from reflection " << std::endl;
        exit(EXIT_FAILURE);        
    }

    google::protobuf::DescriptorPool descPool(&m_reflectionDescDb);
     
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
        const grpc::protobuf::FileDescriptor * serviceFileDesc;
        serviceFileDesc = serviceDesc->file();
        if (serviceFileDesc == nullptr)
        {
            std::cerr << "Service file decriptor for service " << serviceName << "is NULL" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Retrieve all proto files used by the service
        int dependencyCounter = serviceFileDesc->dependency_count();

        m_descNames.insert(serviceFileDesc->name());

        //TODO: Loop only, if dependencies weren't already fetched (If parent not yet in descNames)?
        for (int i=0; i<dependencyCounter; i++){
            const grpc::protobuf::FileDescriptor * dependencyDesc = serviceFileDesc->dependency(i);
            if(dependencyDesc==nullptr)
            {
                std::cerr << "File decriptor for imported file with dependecy count " << i << " is NULL" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Get file descriptor of imported files used in this service and search for more files
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
            m_descNames.insert(currentFileName);
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
        if (host.hostaddress() == f_hostAddress)
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
        if (host.hostaddress() == f_hostAddress)
        {
            for(int j=0; j < host.servicelist_size(); j++)
            {
                serviceName = host.servicelist(j); 
                (m_serviceList).push_back(serviceName);
            }
            break;
        }     
    }
}

void DescDbProxy::getDescriptors(const std::string &f_hostAddress)
{  
    // Prepare DB-File
    std::string homeFolder = getenv("HOME");
    std::string cacheFolderPath = homeFolder + "/.cache/gwhisper";
    std::string dbFileName = "DescriptorCache.bin";
    std::string dbStatus = gwhisper::util::createFolder(cacheFolderPath);
    if(dbStatus == "FAIL")
    {
        std::cerr << "Failed to access folder to store cache at "<< cacheFolderPath << std::endl;
        exit(EXIT_FAILURE);
    }

    // Import .prot file
    localDescDb::DescriptorDb dbFile;
    std::string dbFilePath = cacheFolderPath + "/" + dbFileName;
    std::fstream input;
    input.open(dbFilePath, std::ios::in | std::ios::binary);
    if(!input)
    {
        std::cerr << "Cannot open file, file does not exist. Creating new file.." << std::endl;
    }
    else if (!dbFile.ParseFromIstream(&input)) 
    {
      std::cerr << "Failed to parse local Descriptor Cache." << std::endl;
      exit(EXIT_FAILURE);
    }

    /*if (input.fail()){
        std::cerr<<"Failed to open "<< dbFilePath << ": "<<std::strerror(errno)<<std::endl;
        exit(EXIT_FAILURE);
    }*/

    std::string gwhisperBuildVersion = GWHISPER_BUILD_VERSION;
    bool accessedReflectionDb = false;

    // Add/Update DB entry for new/outdated host (via Reflection)
    if(!isValidHostEntry(dbFile, f_hostAddress) || (dbFile.gwhisper_version() != gwhisperBuildVersion))
    {
        deleteDuplicateHostEntries(dbFile, f_hostAddress, dbFileName);
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
        writeProtoMsgToDisk(dbFile, dbFilePath);
    }
}

DescDbProxy::DescDbProxy(bool disableCache, std::string hostAddress, std::shared_ptr<grpc::Channel> channel) :
    m_reflectionDescDb(channel)
{
    if(disableCache)
    {
        // Get Desc directly via reflection without touching localDB
        fetchDescNamesFromReflection();
        for(auto &name:(m_descNames))
        {
            google::protobuf::FileDescriptorProto descriptor;
            m_reflectionDescDb.FindFileByName(name, &descriptor);
            m_localDB.Add(descriptor);
        }     

    }
    else
    {
        getDescriptors(hostAddress); //channel); 
    }
    
}

DescDbProxy::~DescDbProxy(){}