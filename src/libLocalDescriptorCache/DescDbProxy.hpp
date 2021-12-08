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
#pragma once

#include<string>
#include<time.h>

#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
//#include "gRPC_utils/reflection.grpc.pb.h"
#include "LocalDescDb.pb.h"
#include <grpcpp/impl/codegen/config_protobuf.h>
//#include <libCli/ConnectionManager.hpp>
//#TODO: Generate with protoc: include "LocalDescDb.pb.h"

class DescDbProxy : public grpc::protobuf::DescriptorDatabase{
    public:

    // Find a file by file name.  Fills in in *output and returns true if found.
    // Otherwise, returns false, leaving the contents of *output undefined.
    // TODO: At function call, iterate over all servives in service list. Call function in getDbFromFile()
    virtual bool FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output) override;

    // const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);
    // Find the file that declares the given fully-qualified symbol name.
    // If found, fills in *output and returns true, otherwise returns false
    // and leaves *output undefined.
    virtual bool FindFileContainingSymbol(const std::string& symbol_name,  grpc::protobuf::FileDescriptorProto* output) override;

    // Find the file which defines an extension extending the given message type
    // with the given field number.  If found, fills in *output and returns true,
    // otherwise returns false and leaves *output undefined.  containing_type
    // must be a fully-qualified type name.
    virtual bool FindFileContainingExtension(const std::string& containing_type, int field_number,
                                            grpc::protobuf::FileDescriptorProto* output) override;


    // Lookup Services in Local DescDB. Services have been written into local DescDB
    // at first creation or at the update of DB entry for host. 
    // If services are found, fills in *output and returns true, , otherwise returns
    // false and leaves *output undefined.
    bool GetServices(std::vector<grpc::string>* output); //, const std::string hostAddress
    
    // Checks, if local DB contains valid descriptorDB entries for host
    bool isValidHostEntry(const localDescDb::DescriptorDb& descDb, const std::string hostAddress);

    // Add new entry on local DB for new host address or update outdated entries
    void editLocalDb(localDescDb::Host* host, std::string hostAddress, std::shared_ptr<grpc::Channel> channel);
    
    
    // Instead of loading descriptors from ReflectionDB on the gRPC server, load them from local DB, if the local DB is not outdated..
    // std::shared_ptr<grpc::protobuf::SimpleDescriptorDatabase> loadDbFromFile(std::string dbFileName, std::string hostAddress);
    void loadDbFromFile(std::string dbFileName, std::string hostAddress, std::shared_ptr<grpc::Channel> channel);
    //std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> loadDbFromFile(std::string dbFileName, std::string hostAddress);


    // Stores DescDB acquired via sever reflection locally as protofile (proto3) (might be serialized i.e. not human readable)
    // For each host one protofile
    // Timestamp needed for validity / freshness of this local cache
    std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> writeCacheToFile (std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> f_descDB);


    
    DescDbProxy(std::string dbFileName, std::string hostAddress, std::shared_ptr<grpc::Channel> channel);

    ~DescDbProxy();


    private:

    std::string m_serverAddress;
    //cli::ConnList m_connList;


    //std::shared_ptr<grpc::protobuf::SimpleDescriptorDatabase> localDB;
    grpc::protobuf::SimpleDescriptorDatabase localDB;
    grpc::ProtoReflectionDescriptorDatabase reflectionDescDb;
    //TODO: think about pointer
    std::vector<grpc::string> serviceList;
    std::vector<grpc::string> fileList;
    std::vector<grpc::protobuf::FileDescriptor>descList;
        

};

