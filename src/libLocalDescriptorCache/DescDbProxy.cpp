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
        bool foundViaReflection = (reflectionDescDb.FindFileByName(filename, output));
        if (foundViaReflection){
            // TODO: Fetch ALL descriptors
            localDB.Add(*output);
            // muss ich hier jetzt nicht in meinem LocalDb Protofile Array hosts um einen host ergänzen? 
            // und den FileDesc output in mein DBDesc schreiben?
            return true;
        }
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


    
bool containsHost(const localDescDb::DescriptorDb& descDb, const std::string hostAddress){
    bool containsHost=false;

    for (int i=0; i < descDb.hosts_size(); i++){
        const localDescDb::Host host = descDb.hosts(i);
        if (host.hostaddress() == hostAddress)
        {
            containsHost = true; 
            // Descriptor for complete Host
            //const google::protobuf::Descriptor* descriptor =  host.GetDescriptor();           
        }

    }
    return containsHost;
}

void addEntry(localDescDb::Host* host, std::string hostAddress){
    google::protobuf::Timestamp timestamp;
    //TODO: Set timestamp
    host->set_allocated_lastupdate(&timestamp);
    host->set_hostaddress(hostAddress);
    // TODO: get FileDesc via Reflection

    std::cout << "added new host entry" << std::endl;
    
}

// Instead of loading descriptors from ReflectionDB on the gRPC server, load them from local DB, if the local DB is not outdated..
std::shared_ptr<grpc::protobuf::SimpleDescriptorDatabase> DescDbProxy::getDbFromFile(std::string dbFileName, std::string hostAddress){
    localDescDb::DescriptorDb dbFile;
    google::protobuf::FileDescriptorProto descriptor;

    // Read the existing db (should be serialized)
    std::fstream input;
    input.open(dbFileName);
    dbFile.ParseFromIstream(&input);

    //Add DB entry for new host
    // TODO: check here timestamp (in contains host)
    if(!containsHost(dbFile, hostAddress)){
        addEntry(dbFile.add_hosts(), hostAddress);
        std::cout << dbFile.DebugString();

    }

    DescDbProxy::localDB;
    //host.file_descriptor_proto_;

    // Get Desc for Host
    for (int i=0; i < dbFile.hosts_size(); i++){
        const localDescDb::Host host = dbFile.hosts(i);
        if (host.hostaddress() == hostAddress)
        {
            for (int i=0; i < host.file_descriptor_proto_size(); i++){
                //std::string descriptor =  host.file_descriptor_proto(i);
                //TODO: steht in descriptor jetzt schon der Wert?
                descriptor.ParseFromString(host.file_descriptor_proto(i));
                //google::protobuf::FileDescriptorProto descriptor =  host.file_descriptor_proto(i);
                DescDbProxy::localDB.Add(descriptor);
            }                              
        }
        break;
    }

    //Write updated Db to disc
    std::fstream output(dbFileName, std::ios::out | std::ios::trunc | std::ios::binary);
    dbFile.SerializeToOstream(&output);
}

// Stores DescDB acquired via sever reflection locally as protofile (proto3) (might be serialized i.e. not human readable)
// For each host one protofile
// Timestamp needed for validity / freshness of this local cache
std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> DescDbProxy::writeCacheToFile (std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> f_descDB){
        //Ablauf: Find File by Name --> should return local DB proto-file and descriptor files
        // In the protofile:  Array of hosts (as messages), each consisting of a timestamp, a name and descriptors
        // Then: compare timestamp and if file (entry)  for host fresh enough (or newly created)
        // Save descriptors in localDB (simlpeDB)
        // Do RPC
        // write localDB to localDBFile with ggf. new timestamp




        //Add newmessage to Proto File with  hostname and time Stamp
        // then add descriptors from descDB  to protofile
        // std::string hostname = m_serverAddress;
        //std::string timestamp = ctime(0); // besser mit Chrono. Wie chrono in protobuf? --> protobuf hat 


        // Or maybe via AddFileFromResponse as message --> Wo kriege ich die Response her? : FileDescriptorResponse.descriptor() --> reutns GetDecriptor()

        //send response to db file
}



DescDbProxy::DescDbProxy(std::string dbFileName, std::string hostAddress, std::shared_ptr<grpc::Channel> channel) :
    reflectionDescDb(channel) //channel instanz --> über Argumente von Konstruktor
// grpc::ProtoReflectionDescriptorDatabase(std::unique_ptr<reflection::v1alpha::ServerReflection::Stub> stub)
{
    getDbFromFile(dbFileName, hostAddress);   
}

// DescDbProxy(std::string dbFileName, std::string hostAddress){
    //   getDbFromFile(dbFileName, hostAddress); // Local DB als .proto speichern
    //}

DescDbProxy::~DescDbProxy(){
        //writeCachetoFile(); //Über Desc iterieren --> bekommt man wie in GrammarConstruction
        //  const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);

}





//grpc::protobuf::ProtoReflectionDescriptorDatabase
//grpc::protobuf::DescriptorDatabase()





