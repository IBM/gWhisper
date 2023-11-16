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

#include <string>
#include <time.h>
#include <set>
#include <optional>

#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
#include "libCli/libCli/ConnectionManager.hpp"
#include "libArgParse/ArgParse.hpp"

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
    /// with the given field number.  If found, fills in *output and returns true,
    /// otherwise returns false and leaves *output undefined.
    virtual bool FindFileContainingExtension(const std::string& containing_type, int field_number,
                                            grpc::protobuf::FileDescriptorProto* output) override;

    /// Fetch service list offered by cache / reflection
    std::vector<grpc::string> GetServices(void); 
    
    /// Acts as an Proxy, that loads descriptors from local DB file, if the local DB is not outdated, 
    /// instead of  fetching them via reflection on the gRPC server.
    /// Add/Update host entry to local DB file, if the entry does not exist / is outdated.
    /// Stores DescDB acquired via sever reflection locally as a DB file in proto3 structure.
    /// @param hostAdress Address to the current host 
    void getDescriptors(const std::string &hostAddress);

    /// @brief close the DescDb stream with a given deadline. If the dealine is not set it waits for the stream to close indefinitely.
    /// @param deadline optional deadline to close the DescDb stream.
    /// @return return grpc status as a result of call the finish() on the DescDb stream.
    grpc::Status closeDescDbStream(std::optional<std::chrono::time_point<std::chrono::system_clock>> deadline);
   
    DescDbProxy(bool disableCache, const std::string &hostAddress, std::shared_ptr<grpc::Channel> channel, ArgParse::ParsedElement &parseTree);

    ~DescDbProxy();

    static inline google::protobuf::int64 const m_cacheTimeoutInSeconds = 120;

    private:

    /// Check freshness of local DB file (cache), i.e. if it contains valid descriptor
    /// entries for a host. A host etry if valid, if an entry for the hostadress which
    // is not older than 120 seconds exists.
    bool isValidHostEntry(const localDescDb::DescriptorDb &descDb, const std::string &hostAddress);

    /// Add new/updated host entry for new/outdated entries to cache representation in memory.
    /// @param out_host Host entry, that is filled in this function.
    /// @param hostAddress Address used for new host entry.
    void repopulateLocalDb(localDescDb::Host& out_host, const std::string &hostAddress);

    /// Check, if channel is established when calling via reflection
    /// @param f_hostAddress Address of host to connect to
    void useReflection(const std::string &f_hostAddress);

    /// Retrieves Names of all file descriptors related to any available service 
    /// provided by a grpc server and writes them to m_descNames.
     /// @param f_hostAddress Address of host to fetch services from
    void fetchDescNamesFromReflection(const std::string &hostAddress);

    /// Recursively looks up all file descriptors that are imported by the parentDesc and add their 
    /// names to m_descNames.
    /// @param parentDesc Root file descriptor to start the lookup from.
    void getDependencies(const grpc::protobuf::FileDescriptor &parentDesc);

    /// Writes representation of proto host message in memory into SimpleDescDb object. 
    /// @param dbProtoFile Representation of cache in memory.
    /// @param hostAddress Address of host that is to be converted into a SimpleDescDb object.
    void convertHostEntryToSimpleDescDb(localDescDb::DescriptorDb dbProtoFile, const std::string &hostAddress);

    /// Fetches service Names from local cache (not reflection!) and adds the names to m_serviceList.
    /// @param dbProtoFile proto representation of cache in memory.
    /// @param hostAddress Address of host whose services are retrieved.
    void addServicNamesToserviceList(localDescDb::DescriptorDb dbProtoFile, const std::string &hostAddress);

    /// Checks, if location for cache exists.
    /// If not, creates new folder 'gwhisper' at home/.cache 
    /// @return Path to cache file on file system
    static std::string prepareCacheFile();

    grpc::protobuf::SimpleDescriptorDatabase m_localDB;
    std::unique_ptr<grpc::ProtoReflectionDescriptorDatabase> m_reflectionDescDb = nullptr;
    std::shared_ptr<grpc::Channel> m_channel = nullptr;
    ArgParse::ParsedElement m_parseTree;

    std::vector<const grpc::protobuf::FileDescriptor*>m_descList;
    std::set<std::string> m_descNames;
    std::vector<grpc::string> m_serviceList;
    bool m_disableCache;
};

