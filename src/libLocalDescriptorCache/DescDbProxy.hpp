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

#pragma once

#include<string>
#include<time.h>
#include<set>

#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>

#include "LocalDescDb.pb.h"
#include <grpcpp/impl/codegen/config_protobuf.h>

/// This class acts as an wrapper around the ReflectionDescDb
/// It implements local caching to reduce fetching data via reflection,
/// by writing in a local file.
class DescDbProxy : public grpc::protobuf::DescriptorDatabase{
    public:

    /// Find a file by file name.  Fills in in *output and returns true if found.
    /// Otherwise, returns false, leaving the contents of *output undefined.
    virtual bool FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output) override;

    /// Find the file that declares the given fully-qualified symbol name.
    /// If found, fills in *output and returns true, otherwise returns false
    /// and leaves *output undefined.
    virtual bool FindFileContainingSymbol(const std::string& symbol_name,  grpc::protobuf::FileDescriptorProto* output) override;

    /// Find the file which defines an extension extending the given message type
    // with the given field number.  If found, fills in *output and returns true,
    // otherwise returns false and leaves *output undefined.  containing_type
    // must be a fully-qualified type name.
    virtual bool FindFileContainingExtension(const std::string& containing_type, int field_number,
                                            grpc::protobuf::FileDescriptorProto* output) override;

    // Lookup Services in Local DescDB. Services have been written into local DescDB
    // at first creation or at the update of DB entry for host. 
    // If services are found, fills in *output and returns true, otherwise returns
    // false and leaves *output undefined.
    bool GetServices(std::vector<grpc::string>* output); 
    
    /// Acts as an Proxy, that loads descriptors from local DB file, if the local DB is not outdated, 
    /// instead of  fetching them via reflection on the gRPC server.
    /// Add/Update host entry to local DB file, if the entry does not exist / is outdated.
    /// Stores DescDB acquired via sever reflection locally as a DB file in proto3 structure.
    /// @param dbFileName Name of file that serves as local chache
    /// @param hostAdress Address to the current host 
    /// @param channel
    void getDescriptors(const std::string &hostAddress); //std::shared_ptr<grpc::Channel> channel);
   
    DescDbProxy(bool disableCache,  std::string hostAddress, std::shared_ptr<grpc::Channel> channel);

    ~DescDbProxy();

    private:

    /// Check freshness of local DB file (cache), i.e. if it contains valid descriptor
    /// entries for a host
    bool isValidHostEntry(const localDescDb::DescriptorDb& descDb, const std::string hostAddress);

    /// 
    void getDependencies(const grpc::protobuf::FileDescriptor * parentDesc);

    /// Add new/updated host entry for new/outdated entries to cache
    /// @param host Host entry, that is filled in this function
    /// @param hostAddress
    void repopulateLocalDb(localDescDb::Host* host, const std::string &hostAddress);// std::shared_ptr<grpc::Channel> channel);

    /// Retrieves Names of service descriptors as well as the names of descriptors the service descriptors depend on
    /// TODO: Better Docu
    void fetchDescNamesFromReflection();

    /// Writes representation of proto host message in memory into SimpleDescDb object. 
    /// @param accessedReflectionDb Flag to show, whether data was accessed via reflection or not
    /// @param dbProtoFile Representation of cache in memory
    /// @param hostAddress 
    void convertHostEntryToSimpleDescDb(bool accessedReflectionDb, localDescDb::DescriptorDb dbProtoFile, std::string hostAddress);

    // std::string m_serverAddress;
    grpc::protobuf::SimpleDescriptorDatabase m_localDB;
    grpc::ProtoReflectionDescriptorDatabase m_reflectionDescDb;

    std::vector<const grpc::protobuf::FileDescriptor*>m_descList;
    std::set<std::string> m_descNames;
    std::vector<grpc::string> m_serviceList;
};

