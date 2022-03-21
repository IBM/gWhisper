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

bool DescDbProxy::GetServices(std::vector<grpc::string>& output)
{
    if(! m_serviceList.empty())
    { 
        output = m_serviceList;
        return true;
    } 
    else
    {
        std::cerr << "Error in DescDbProxy while fetching service"  << std::endl;
        return false;    
    }
}  

bool DescDbProxy::isValidHostEntry(const localDescDb::DescriptorDb& f_descDb, const std::string f_hostAddress)
{    
    bool validHostEntry=false; //TODO: Maybe rename in something with Reflection
    //static google::protobuf::util::TimeUtil timestamp;
    //google::protobuf::Timestamp currentTime;
    //google::protobuf::Timestamp lastUpdate;
    //google::protobuf::int64 secCurrentTime;
    //google::protobuf::int64 secLastUpdate;
    //int timeDiff;

    for (int i=0; i < f_descDb.hosts_size(); i++)
    {
        const localDescDb::Host host = f_descDb.hosts(i);
        if (host.hostaddress() == f_hostAddress)
        {
            google::protobuf::util::TimeUtil timestamp;
            // Get current Time and Time of last Upate in Seconds:
            google::protobuf::Timestamp currentTime = timestamp.GetCurrentTime(); // time in UTC
            google::protobuf::int64 secCurrentTime = timestamp.TimestampToSeconds(currentTime);
            
            google::protobuf::Timestamp lastUpdate = host.lastupdate();
            google::protobuf::int64 secLastUpdate = timestamp.TimestampToSeconds(lastUpdate);

            google::protobuf::int64 timeDiff = secCurrentTime - secLastUpdate;

            // Expiry for validity of an entry
            if (timeDiff <= 120)
            {
                validHostEntry = true;
            }      
        }
    }

    return validHostEntry;
}

/// Helper class to write representation of a proto file in RAM to Disk.
/// @param f_protoFile Proto message that is written to disk.
/// @param f_outputFile Name of file this function writes in.
void writeProtoMsgToDisk(localDescDb::DescriptorDb& f_protoFile, std::string f_outputFile){
    std::fstream output(f_outputFile, std::ios::out | std::ios::binary);
    if (!f_protoFile.SerializeToOstream(&output)){
        std::cerr << "Failed to write "<< f_outputFile << std::endl;
    }
}

// Remove all instances in DB od same host
// Writes all other hosts in new messages and retuns this new message into descDb variable
void deleteDuplicateHostEntries(localDescDb::DescriptorDb& out_descDb, const std::string f_hostAddress, const std::string f_dbFile)
{
    localDescDb::DescriptorDb newDescDb;
    for(int i=0; i<out_descDb.hosts_size(); i++)
    {
        localDescDb::Host host = out_descDb.hosts(i);
        if(host.hostaddress() != f_hostAddress)
        {
            localDescDb::Host* newHost = newDescDb.add_hosts(); //returns host
            newHost->CopyFrom(host);
        }
    }

    out_descDb = newDescDb;
}


void DescDbProxy::repopulateLocalDb(localDescDb::Host& out_host, const std::string &f_hostAddress)//,std::shared_ptr<grpc::Channel> f_channel)
{
    static google::protobuf::util::TimeUtil timestamp;
    localDescDb::Host* host = &out_host;

   /* if(!host){
        std::cerr<<"New host entry is nullpointer"<<std::endl;
    }*/

    fetchDescNamesFromReflection();

    //TODO Neue Funktion: AddEntry()? Ginge das dann mit Auslagerung in neue Klasse?
    host->set_hostaddress(f_hostAddress);
    (*(host->mutable_lastupdate())) = timestamp.GetCurrentTime();
    // Add service to loacal DB
    for(const auto& serviceName: (m_serviceList))
    { 
        host->add_servicelist(serviceName); 
    }

    // Add all descriptors to DB entry
    for (const auto& fileName: (m_descNames))
    {
        grpc::protobuf::FileDescriptorProto  output;
        m_reflectionDescDb.FindFileByName(fileName, &output);
        std::string dbDescEntry = output.SerializeAsString();
        host->add_file_descriptor_proto(dbDescEntry); 
    }
}

void DescDbProxy::fetchDescNamesFromReflection()
{
    m_reflectionDescDb.GetServices(&m_serviceList);
    google::protobuf::DescriptorPool descPool(&m_reflectionDescDb);
     
    for(const auto& serviceName: (m_serviceList)){
        //const grpc::protobuf::FileDescriptor * serviceFileDesc;
        //const grpc::protobuf::FileDescriptor * dependencyDesc;
        // const grpc::protobuf::ServiceDescriptor * serviceDesc;

        // Get service file names through services in descPool
        const grpc::protobuf::ServiceDescriptor * serviceDesc;
        serviceDesc = descPool.FindServiceByName(serviceName);
        // Catch nullptr, if services does not have a descriptor
        if (serviceDesc == nullptr)
        {
            continue;
        }

        // Use filenames to retrieve service descriptors from ReflectionDB
        const grpc::protobuf::FileDescriptor * serviceFileDesc;
        serviceFileDesc = serviceDesc->file();
        if (serviceFileDesc == nullptr) // can this happen? serviceDesc has always a file name
        {
            continue;
        }

        // Retrieve all proto files used by the service
        int dependencyCounter = serviceFileDesc->dependency_count();
        m_descNames.insert(serviceFileDesc->name());

        //TODO: Loop only, if dependencies weren't already fetched (If parent not yet in descNames)?
        for (int i=0; i<dependencyCounter; i++){
            const grpc::protobuf::FileDescriptor * dependencyDesc = serviceFileDesc->dependency(i);
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

void DescDbProxy::convertHostEntryToSimpleDescDb(bool f_accessedReflectionDb, localDescDb::DescriptorDb f_dbProtoFile, std::string f_hostAddress)
{
    // Add services to localDB variable if DescDb on file was valid
    std::string serviceName;
    if (!f_accessedReflectionDb)
    {
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
            }     
        }
    }

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

void DescDbProxy::getDescriptors(const std::string &f_hostAddress)
{  
    // Prepare DB-File
    std::string homeFolder = getenv("HOME");
    std::string cacheFolderPath = homeFolder + "/.cache/gwhisper";
    std::string dbFileName = "LocalDescDb.bin";
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
    input.open(dbFilePath);
    if (input.fail()){
        std::cerr<<"Failed to open "<< dbFilePath << ": "<<std::strerror(errno)<<std::endl;
    }

    dbFile.ParseFromIstream(&input); 

    std::string gwhisperBuildVersion = GWHISPER_BUILD_VERSION;
    bool accessedReflectionDb = false;

    // Add/Update DB entry for new/outdated host (via Reflection)
    if(!isValidHostEntry(dbFile, f_hostAddress) || (dbFile.gwhisper_version() != gwhisperBuildVersion))
    {
        deleteDuplicateHostEntries(dbFile, f_hostAddress, dbFileName);
        writeProtoMsgToDisk(dbFile, dbFileName); 
        dbFile.clear_gwhisper_version();
        dbFile.set_gwhisper_version(gwhisperBuildVersion);
        repopulateLocalDb(*(dbFile.add_hosts()), f_hostAddress);
        accessedReflectionDb = true;
    }

    convertHostEntryToSimpleDescDb(accessedReflectionDb, dbFile, f_hostAddress);

    // Overwrite DB file on disc
    /*std::fstream output(dbFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!dbFile.SerializeToOstream(&output))
    {
        std::cerr << "Failed to write DB." << std::endl;
    }*/

    writeProtoMsgToDisk(dbFile, dbFileName); 

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