// Copyright 2022 IBM Corporation
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

#include <gtest/getest.h>
#include <grpcpp/grpcpp.h>
#include <gRPC_utils/proto_reflection_descriptor_database.h>
// -----------------------------------------------------------------------------
//          Caching
// -----------------------------------------------------------------------------

// Build Stub for channel
// --> Predefining output for channel
// I can't really do this--> How to build cahnnel?

// Build Stub for reflectionDB
// --> Predefining Output for Interface of reflectionDB
// How to pass reflectionDB? --> I need reflection(channel)

// Stubs in gtests? --> Mock a stub
// TODO: Create fully controllabel DescDB
// Wrapper onject for channel?

// Is this a Mock or a stub?
//virtual mockProtoReflectionDescriptorDatabase : grpc::ProtoReflectoinDescriptorDatabase
// Override Methodss
//
//

// TODO: Maybe use existing grpc Mock-Server --> Difference to Test-server?y

TEST(CacheTest, DBError){
    grpc::ProtoReflectionDescriptorDatabase protoDescDb; // Do need to mock this too?
    std::shared_ptr<grpc::Channel> channel; // I need mock Channel

    // Call Proxy with the above defined parameters
    // Make assertions about expected behaviour
}

TEST(CacheTest, EmptyServiceList){
    // Define a Proxy

    // Set empty Testserver Service List 
    // Proxy.getServices()

    //Expect: Return Empty ServiceList

}

TEST(CacheTest, EmptyServiceList){
    // Define a Proxy

    // Set invalid Testserver Service List 
    // Proxy.getServices()

    //Expect: Return False

}

TEST(cacheTest, MultipleCallServiceList){
    // Define a Proxy

    // Set invalid Testserver Service List 
    // Proxy.getServices()
    // Proxy.getServices()

    //Expect: 2x same Service List
}


