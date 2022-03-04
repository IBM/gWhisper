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


/// Check wether get DescDB from local location or via server reflection
/// Input: server Adress, connection List (or 1 Param if possible with connections.[server_adress])
/// Output: Db either of type local or type refectoin. Local should have same interface as reflection (take reflection as reference)
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

bool DescDbProxy::FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output){
    return m_localDB.FindFileByName(filename, output);                 
}

bool DescDbProxy::FindFileContainingSymbol(const std::string& symbol_name,
                                        grpc::protobuf::FileDescriptorProto* output){
    return m_localDB.FindFileContainingSymbol(symbol_name, output);                 
}                              

bool DescDbProxy::FindFileContainingExtension(const std::string& containing_type,
                                        int f_field_number,
                                        grpc::protobuf::FileDescriptorProto* output){
    return m_localDB.FindFileContainingExtension(containing_type, f_field_number, output);                 
}       

bool DescDbProxy::GetServices(std::vector<grpc::string>* output){ //

   if(! m_serviceList.empty()){
      // output = serviceList; TODO here:pushback?
      (*output) = m_serviceList;
       return true;

   } else {
        //Wie loggen wir?
        std::cout << "Error while fetching services from localDB entry for host "  << std::endl;
        return false;    
   }
}  



bool DescDbProxy::isValidHostEntry(const localDescDb::DescriptorDb& f_descDb, const std::string f_hostAddress){
    
    bool validHostEntry=false; //TODO: Maybe rename in something with Reflection
    static google::protobuf::util::TimeUtil timestamp;
    //static google::protobuf::util::TimeUtil lastUpdateTime;
    google::protobuf::Timestamp currentTime;
    google::protobuf::Timestamp lastUpdate;
    int secCurrentTime;
    int secLastUpdate;
    int timeDiff;

    for (int i=0; i < f_descDb.hosts_size(); i++){
        const localDescDb::Host host = f_descDb.hosts(i);
        if (host.hostaddress() == f_hostAddress)
        {
           // Get current Time and Time of last Upate in Seconds:
            currentTime = timestamp.GetCurrentTime(); // time in UTC
            secCurrentTime = timestamp.TimestampToSeconds(currentTime);
            
            lastUpdate = host.lastupdate();
            secLastUpdate = timestamp.TimestampToSeconds(lastUpdate);

            // Calculate time in seconds btw. lastUpdate and currentTime
            timeDiff = secCurrentTime - secLastUpdate;

            if (timeDiff <= 120){
                validHostEntry = true;
            }      
        }

    }
    return validHostEntry;
}

void deleteDuplicateHostEntries(const localDescDb::DescriptorDb& descDb, const std::string f_hostAddress){
    for(int i=0; i<descDb.hosts_size(); i++){
        localDescDb::Host host = descDb.hosts(i);
        if(host.hostaddress() == f_hostAddress){
            host.clear_hostaddress();
            host.clear_lastupdate();
            host.clear_servicelist();
            host.clear_file_descriptor_proto();
        }

    }

}

void DescDbProxy::repopulateLocalDb(localDescDb::Host* host, std::string f_hostAddress,std::shared_ptr<grpc::Channel> f_channel){

    static google::protobuf::util::TimeUtil timestamp;

    //TODO: Duplicates during fetching or as post-processing 
    //(when write into DB variable, before writing to disk)
    fetchDescNamesFromReflection();

    //TODO Neue Funktion: AddEntry():
    host->set_hostaddress(f_hostAddress);
    //lastUpdate->set_seconds(5); 
    (*(host->mutable_lastupdate())) = timestamp.GetCurrentTime();
    // Add service Naems to DB entry
    for(const auto& serviceName: (m_serviceList)){
        // Add service to loacal DB
        host->add_servicelist(serviceName); 
    }

    // Add all descriptors to DB entry
    // m_ vor membern
    for (const auto& fileName: (m_descNames)){
        grpc::protobuf::FileDescriptorProto  output;
        m_reflectionDescDb.FindFileByName(fileName, &output);
        std::string dbDescEntry = output.SerializeAsString();
        host->add_file_descriptor_proto(dbDescEntry); 
    }
}

void DescDbProxy::fetchDescNamesFromReflection(){

    // TODO: Wie weiß Reflection DB auf welchen Host sie muss?
    m_reflectionDescDb.GetServices(&m_serviceList);
    google::protobuf::DescriptorPool descPool(&m_reflectionDescDb);
     
    for(const auto& serviceName: (m_serviceList)){
        const grpc::protobuf::FileDescriptor * serviceFileDesc;
        const grpc::protobuf::FileDescriptor * dependencyDesc;
        const grpc::protobuf::ServiceDescriptor * serviceDesc;

        // Get service file names through services in descPool
        // Use filenames to retrieve service descriptors from ReflectionDB
        serviceDesc = descPool.FindServiceByName(serviceName);
        if (! serviceDesc){
            // Catching Nullpointer wenn service keinen desc hat
            continue;
        }

        serviceFileDesc = serviceDesc->file();

        // Retrieve all proto files used by the service
        int dependencyCounter = serviceFileDesc->dependency_count();

        //m_descNames.push_back(serviceFileDesc->name());
        m_descNames.insert(serviceFileDesc->name());

        //TODO: Loop only, if dependencies weren't already fetched (If parent not yet in descNames)?
        for (int i=0; i<dependencyCounter; i++){
            dependencyDesc = serviceFileDesc->dependency(i);
            // Get file of imported files used in this service and search for more files
            getDependencies(dependencyDesc);
        }
    }
}

void DescDbProxy::getDependencies(const grpc::protobuf::FileDescriptor * f_parentDesc){
        
        std::deque<const grpc::protobuf::FileDescriptor*> todoList;
        std::vector<const grpc::protobuf::FileDescriptor*> doneList;
        int amountChildren;

        todoList.push_front(f_parentDesc);

        while (not todoList.empty()){
            if(std::find(doneList.begin(), doneList.end(),todoList.front()) != doneList.end()){
                // Discard already processed descriptors
                todoList.pop_front();
            }else{
                amountChildren = todoList.front()->dependency_count();
                for (int c=0; c < amountChildren; c++)
                {    
                    todoList.push_back(todoList.front()->dependency(c));
                }
        
                //const grpc::protobuf::FileDescriptor* test = todoList.front();
                std::string currentFileName = todoList.front()->name();  
                //m_descNames.push_back(currentFileName);
                m_descNames.insert(currentFileName);
                doneList.push_back(todoList.front());
                todoList.pop_front();    
            }          
        }       
}

void DescDbProxy::convertHostEntryToSimpleDescDb(bool f_accessedReflectionDb, localDescDb::DescriptorDb f_dbProtoFile, std::string f_hostAddress){
    // Add services to localDB variable if not happened in editLocalDB
    std::string serviceName;
    if (!f_accessedReflectionDb){
        for (int i = 0; i < f_dbProtoFile.hosts_size(); i++){
            const localDescDb::Host host = f_dbProtoFile.hosts(i); 
            if (host.hostaddress() == f_hostAddress){
                for(int j=0; j < host.servicelist_size(); j++){
                    serviceName = host.servicelist(j); 
                    (m_serviceList).push_back(serviceName);
                }
            }     
        }
    }

    // Get Desc for Host and add them to simpleDescDb object m_localDB
    // !! host 50043 wird nicht dazugepackt! --> steht och nicht drin (warum nicht? die 50053 schaffens ja auch rein)
    for (int i=0; i < f_dbProtoFile.hosts_size(); i++){
        const localDescDb::Host host = f_dbProtoFile.hosts(i);
        if (host.hostaddress() == f_hostAddress)
        {
            for (int i=0; i < host.file_descriptor_proto_size(); i++){
                google::protobuf::FileDescriptorProto descriptor;
                google::protobuf::FileDescriptorProto output;
                //std::string descriptor =  host.file_descriptor_proto(i);
                //Here: Error "File already exists in DB", google/protobuf/descriptor_database.cc:120
                //TODO: prevent Duplicates here! (File-already exists-Error) --> better prevent Duplicates in reflection
                descriptor.ParseFromString(host.file_descriptor_proto(i));
                if (!m_localDB.FindFileByName(descriptor.name(), &output)){
                    m_localDB.Add(descriptor);
                }
            }   
            break;                    
        }
    }
}


void DescDbProxy::getDescriptors(std::string f_dbFileName, std::string f_hostAddress, std::shared_ptr<grpc::Channel> f_channel){
    
    //Use dbFile from .prot file
    localDescDb::DescriptorDb dbFile;
    // Read the existing DB on disk
    std::fstream input;
    input.open(f_dbFileName);
    dbFile.ParseFromIstream(&input); 

    //Add/Update DB entry for new/outdated host (via Reflection)
    std::string gwhisperBuildVersion = GWHISPER_BUILD_VERSION;
    bool accessedReflectionDb = false;
    if(!isValidHostEntry(dbFile, f_hostAddress) || (dbFile.gwhisper_version() != gwhisperBuildVersion)){
        //TODO: Delete host Entry before writing new
        dbFile.clear_gwhisper_version();
        dbFile.set_gwhisper_version(gwhisperBuildVersion); //Correct Place to set version?
        deleteDuplicateHostEntries(dbFile, f_hostAddress);
        repopulateLocalDb(dbFile.add_hosts(), f_hostAddress, f_channel);
        accessedReflectionDb = true;
        //std::cout << dbFile.DebugString();
    }

    convertHostEntryToSimpleDescDb(accessedReflectionDb, dbFile, f_hostAddress);

   // writeCacheToFile(dbFileName, dbFile, localDescDb);
    //Write updated Db to disc --> new function: write Cahce to File
    std::fstream output(f_dbFileName, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!dbFile.SerializeToOstream(&output)){
        std::cerr << "Failed to write DB." << std::endl;
    }
}


DescDbProxy::DescDbProxy(std::string dbFileName, std::string hostAddress, std::shared_ptr<grpc::Channel> channel) :
    m_reflectionDescDb(channel)
{
    getDescriptors(dbFileName, hostAddress, channel);   //sollte localDB returnen?
}

DescDbProxy::~DescDbProxy(){
        //writeCachetoFile(); //Über Desc iterieren --> bekommt man wie in GrammarConstruction
        //  const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);

}