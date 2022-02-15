// Copyright 2019 IBM Corporation
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

#include "ServiceStreamingRpcs.hpp"
#include <unistd.h>

::grpc::Status ServiceStreamingRpcs::replyStreamEmpty(
        ::grpc::ServerContext* context,
        const ::examples::Uint32* request,
        ::grpc::ServerWriter< ::google::protobuf::Empty>* writer
        )
{
    bool ok = true;
    uint32_t count = 0;
    while((count < request->number()) and ok)
    {
        ::google::protobuf::Empty empty;
        ok = writer->Write(empty);
        count++;
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::replyStreamTimestamp10Hz(
        ::grpc::ServerContext* context,
        const ::examples::Uint32* request,
        ::grpc::ServerWriter< ::google::protobuf::Timestamp>* writer
        )
{

    bool ok = true;
    uint32_t count = 0;
    while((count < request->number()) and ok)
    {
        ::google::protobuf::Timestamp timestamp;
        std::time_t t = std::time(0) ;
        struct tm zero = {0};

        zero.tm_hour    = 0;
        zero.tm_min     = 0;
        zero.tm_sec     = 0;
        zero.tm_year    = 0;
        zero.tm_mon     = 0;
        zero.tm_mday    = 0;

        double seconds = difftime(t, mktime(&zero));

        timestamp.set_seconds(seconds);
        timestamp.set_nanos((seconds- static_cast<uint64_t>(seconds))*1000000000);

        ok = writer->Write(timestamp);
        usleep(100000);
        count++;
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::requestStreamAddAllNumbers(
        ::grpc::ServerContext* context,
        ::grpc::ServerReader< ::examples::Uint32>* reader, ::examples::Uint32* response
        )
{
    ::examples::Uint32 message;
    while (reader->Read(&message)) {
        response->set_number(response->number() + message.number());
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::requestStreamCountMessages(
        ::grpc::ServerContext* context,
        ::grpc::ServerReader< ::google::protobuf::Empty>* reader, ::examples::Uint32* response
        )
{
    ::google::protobuf::Empty message;
    while (reader->Read(&message)) {
        response->set_number(response->number() + 1);
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::bidirectionalStreamNegateNumbers(
        ::grpc::ServerContext* context,
        ::grpc::ServerReaderWriter< ::examples::Int32, ::examples::Int32>* stream
        )
{
    ::examples::Int32 message;
    while (stream->Read(&message)) {
        ::examples::Int32 reply;
        reply.set_number(-message.number());
        stream->Write(reply);
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::bidirectionalStreamEchoNumberOrString(
            ::grpc::ServerContext* context,
            ::grpc::ServerReaderWriter< ::examples::NumberOrStringOneOf,
            ::examples::NumberOrStringOneOf>* stream
            )
{
    ::examples::NumberOrStringOneOf message;
    while (stream->Read(&message)) {
        stream->Write(message);
    }
    return grpc::Status();
}

::grpc::Status ServiceStreamingRpcs::bidirectionalStreamInfiniteRpc(
            ::grpc::ServerContext* context,
            ::grpc::ServerReaderWriter<::google::protobuf::Empty, ::google::protobuf::Empty>* stream
            )
{
    ::google::protobuf::Empty message;
    u_int64_t i = 0;

    while(not context->IsCancelled()){
        sleep(1);
        i++;        
    }
    ::google::protobuf::Empty reply;
    stream->Write(reply);

    std::cout<< "RPCs was cancelled after " << i << " seconds." << std::endl;
    return grpc::Status();   
}
