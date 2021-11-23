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

#include<string>
#include <iostream>
#include <fstream>

#include <google/protobuf/descriptor.h>
#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
#include <google/protobuf/util/time_util.h>
//#include "gRPC_utils/reflection.grpc.pb.h"
#include "LocalDescDb.pb.h"
#include <grpcpp/impl/codegen/config_protobuf.h>
//#include <libCli/ConnectionManager.hpp>
//#TODO: Generate with protoc: include "LocalDescDb.pb.h"



// Find a file by file name.  Fills in in *output and returns true if found.
// Otherwise, returns false, leaving the contents of *output undefined.
// TODO: At function call, iterate over all servives in service list. Call function in getDbFromFile()
bool DescDbProxy::FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output){
    bool foundInFile = localDB.FindFileByName(filename, output);
    if (foundInFile){
        return true;
    } else {
        return false;
    }                               
}

// const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);
// Find the file that declares the given fully-qualified symbol name.
// If found, fills in *output and returns true, otherwise returns false
// and leaves *output undefined.
bool DescDbProxy::FindFileContainingSymbol(const std::string& symbol_name,
                                        grpc::protobuf::FileDescriptorProto* output){}

// Find the file which defines an extension extending the given message type
// with the given field number.  If found, fills in *output and returns true,
// otherwise returns false and leaves *output undefined.  containing_type
// must be a fully-qualified type name.
bool DescDbProxy::FindFileContainingExtension(const std::string& containing_type,
                                        int field_number,
                                        grpc::protobuf::FileDescriptorProto* output){}

// Lookup Services in Local DescDB. Services have been written into local DescDB
// at first creation or at the update of DB entry for host. 
// If services are found, fills in *output and returns true, , otherwise returns
// false and leaves *output undefined.
bool DescDbProxy::GetServices(std::vector<grpc::string>* output, const std::string hostAddress){
   
   // TODO: member-var(verctor) für service list. her nur noch assignen
   // Access right host entry in db --> Put this in load from file.
   if(! serviceList->empty()){
       output = serviceList;
       return true;

   } else {
        //Wie loggen wir?
        std::cout << "Error while fetching services from localDB entry for host " << hostAddress << std::endl;
        return false;    
   }
}  


bool isValidHostEntry(const localDescDb::DescriptorDb& descDb, const std::string hostAddress){
    bool validHostEntry=false; //TODO: Maybe rename in something with Reflection
    static google::protobuf::util::TimeUtil timestamp;
    //static google::protobuf::util::TimeUtil lastUpdateTime;
    google::protobuf::Timestamp currentTime;
    google::protobuf::Timestamp lastUpdate;
    int secCurrentTime;
    int secLastUpdate;
    int timeDiff;

    for (int i=0; i < descDb.hosts_size(); i++){
        const localDescDb::Host host = descDb.hosts(i);
        if (host.hostaddress() == hostAddress)
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

void editLocalDb(localDescDb::Host* host, std::string hostAddress){
    static google::protobuf::util::TimeUtil timestamp;
    google::protobuf::Timestamp currentTime = timestamp.GetCurrentTime(); // current Time in UTC 
    host->set_allocated_lastupdate(&currentTime);
    host->set_hostaddress(hostAddress);
    // TODO: get FileDesc and ServiceList via Reflection
    //reflectionDB.getFilebynae(file, output) //get Desc from reflectionDB
    //write Desc (output) into Proto

    //reflecionDB.getServices() //fetch Services via reflection
    //write each service name into proto

    std::cout << "added new host entry" << std::endl;
    
}

// Instead of loading descriptors from ReflectionDB on the gRPC server, load them from local DB, if the local DB is not outdated..
std::shared_ptr<grpc::protobuf::SimpleDescriptorDatabase> DescDbProxy::loadDbFromFile(std::string dbFileName, std::string hostAddress){
    google::protobuf::FileDescriptorProto descriptor;
    //Use dbFile from .prot file
    localDescDb::DescriptorDb dbFile;
    std::string serviceName;

    // Read the existing db (should be serialized)
    std::fstream input;
    input.open(dbFileName);
    dbFile.ParseFromIstream(&input);

    //Add/Update DB entry for new/outdated host (via Reflection)
    if(!isValidHostEntry(dbFile, hostAddress)){
        editLocalDb(dbFile.add_hosts(), hostAddress);
        std::cout << dbFile.DebugString();

    }


    //Write service names from DB into variable
    for (int i = 0; i < dbFile.hosts_size(); i++){

        const localDescDb::Host host = dbFile.hosts(i); //Todo this access proceess evtl in own method
        if (host.hostaddress() == hostAddress){
            // get Services from DB entry for host 
            for(int j=0; j < host.servicelist_size(); j++){
                serviceName = host.servicelist(i);
                // Save services in memeber variable
                (*serviceList).push_back(serviceName);
            }
        }     
    }



    // Get Desc for Host and add them to localDB
    for (int i=0; i < dbFile.hosts_size(); i++){
        const localDescDb::Host host = dbFile.hosts(i);
        if (host.hostaddress() == hostAddress)
        {
            for (int i=0; i < host.file_descriptor_proto_size(); i++){
                //std::string descriptor =  host.file_descriptor_proto(i);
                //TODO: steht in descriptor jetzt schon der Wert? Ist ein i = ein Descriptor?
                descriptor.ParseFromString(host.file_descriptor_proto(i));
                //google::protobuf::FileDescriptorProto descriptor =  host.file_descriptor_proto(i);
                localDB.Add(descriptor);
            }                              
        }
        break;
    }

   // writeCacheToFile(dbFileName, dbFile, localDescDb);

    //Write updated Db to disc --> new function: write Cahce to File
    std::fstream output(dbFileName, std::ios::out | std::ios::trunc | std::ios::binary);
    dbFile.SerializeToOstream(&output);
}

// Stores DescDB acquired via sever reflection locally as protofile (proto3) (might be serialized i.e. not human readable)
// For each host one protofile
// Timestamp needed for validity / freshness of this local cache
std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> DescDbProxy::writeCacheToFile (std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> f_descDB){
      
}



DescDbProxy::DescDbProxy(std::string dbFileName, std::string hostAddress, std::shared_ptr<grpc::Channel> channel) :
    reflectionDescDb(channel) //channel instanz --> über Argumente von Konstruktor
// grpc::ProtoReflectionDescriptorDatabase(std::unique_ptr<reflection::v1alpha::ServerReflection::Stub> stub)
{
    loadDbFromFile(dbFileName, hostAddress);   
}


DescDbProxy::~DescDbProxy(){
        //writeCachetoFile(); //Über Desc iterieren --> bekommt man wie in GrammarConstruction
        //  const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);

}







