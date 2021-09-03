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
#include<string>
#include<time.h>

#include <grpcpp/grpcpp.h>
#include "third_party/gRPC_utils/reflection.grpc.pb.h"
#include <grpcpp/impl/codegen/config_protobuf.h>
#include <libCli/ConnectionManager.hpp>
//#TODO: Generate with protoc: include "LocalDescDb.pb.h"

class DescDBProxy : public grpc::protobuf::DescriptorDatabase{
    public:

    // Find a file by file name.  Fills in in *output and returns true if found.
    // Otherwise, returns false, leaving the contents of *output undefined.
    // TODO: At function call, iterate over all servives in service list. Call function in getDbFromFile()
    virtual bool FindFileByName(const std::string& filename, grpc::protobuf::FileDescriptorProto* output) override {
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
    virtual bool FindFileContainingSymbol(const std::string& symbol_name,
                                            grpc::protobuf::FileDescriptorProto* output) override {}

    // Find the file which defines an extension extending the given message type
    // with the given field number.  If found, fills in *output and returns true,
    // otherwise returns false and leaves *output undefined.  containing_type
    // must be a fully-qualified type name.
    virtual bool FindFileContainingExtension(const std::string& containing_type,
                                            int field_number,
                                            grpc::protobuf::FileDescriptorProto* output) override {}


    
    // Instead of loading descriptors from ReflectionDB on the gRPC server, load them from local DB, if the local DB is not outdated..
    std::shared_ptr<grpc::protobuf::SimpleDescriptorDatabase> getDbFromFile(std::string dbFileName, std::string hostAddress){
        // TODO: Exectue protoc and install google-protobuf runtime (npm install google-protobuf)
        // In: protofile localDescDB
        // Out: Timestamp + Desc for given host Address
        // Save Desc in localDescDB --> call Find File by Name

        //should be Array of hosts
        const gwhisper.localDescDb::DescriptorDb descriptorDb;
        std::string gwhisper.localDescDb::Host host;



        // Here happens call to proto API
        // TODO: Save Timestamp
        
        // Create Cache:
        // iterate over all services
        //TODO: get List of service names
        ///std::vector<grpc::string> serviceList;
        //f_descDB.getServices(&serviceList);


    }

    // Stores DescDB acquired via sever reflection locally as protofile (proto3) (might be serialized i.e. not human readable)
    // For each host one protofile
    // Timestamp needed for validity / freshness of this local cache
    std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> writeCacheToFile (std::shared_ptr<grpc::ProtoReflectionDescriptorDatabase> f_descDB){
        //Ablauf: Find File by Name --> should return local DB proto-file and descriptor files
        // In the protofile:  Array of hosts (as messages), each consisting of a timestamp, a name and descriptors
        // Then: compare timestamp and if file (entry)  for host fresh enough (or newly created)
        // Save descriptors in localDB (simlpeDB)
        // Do RPC
        // write localDB to localDBFile with ggf. new timestamp




        //Add newmessage to Proto File with  hostname and time Stamp
        // then add descriptors from descDB  to protofile
        std::string hostname = m_serverAddress;
        std::string timestamp = ctime(0); // besser mit Chrono. Wie chrono in protobuf? --> protobuf hat 


        // Or maybe via AddFileFromResponse as message --> Wo kriege ich die Response her? : FileDescriptorResponse.descriptor() --> reutns GetDecriptor()

        //send response to db file
    }



    
    DescDBProxy(std::string dbFileName, std::string hostAddress){
        getDbFromFile(dbFileName, hostAddress); // Local DB als .proto speichern
    }

    ~DescDBProxy(){
        writeCachetoFile(); //Über Desc iterieren --> bekommt man wie in GrammarConstruction
        //  const grpc::protobuf::ServiceDescriptor *service = ConnectionManager::getInstance().getDescPool(serverAddress, *f_parseTree)->FindServiceByName(serviceName);

    }

    std::string m_serverAddress;
    //cli::ConnList m_connList;
    std::unordered_map<std::string, cli::ConnList> m_connections;


    private:
        grpc::protobuf::SimpleDescriptorDatabase localDB;
        grpc::ProtoReflectionDescriptorDatabase reflectionDescDb;
        

};

