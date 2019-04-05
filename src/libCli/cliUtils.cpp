#include "libCli/cliUtils.hpp"

namespace cli
{
    bool waitForChannelConnected(std::shared_ptr<grpc::Channel> f_channel, uint32_t f_timeoutMs)
    {
        gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_MONOTONIC), gpr_time_from_micros(f_timeoutMs*1000, GPR_TIMESPAN));
        bool result = f_channel->WaitForConnected(deadline);
        return result;
    }

    uint32_t getConnectTimeoutMs(ArgParse::ParsedElement * f_parseTree, uint32_t f_default)
    {
        // TODO: it would be nice to encode default values for options in the grammar
        // TODO: also it would be nice to provide grammar/parsed elements which can output c++ types other than strings (integers, etc.) 
        std::string connectTimeoutStr = f_parseTree->findFirstChild("connectTimeout");
        uint32_t connectTimeoutMs = f_default;
        if(connectTimeoutStr != "")
        {
            connectTimeoutMs = std::stol(connectTimeoutStr);
        }
        return connectTimeoutMs;
    }

    std::string getGrpcStatusCodeAsString(grpc::StatusCode f_statusCode)
    {

        static const std::map<grpc::StatusCode, const char*> codeMap{
            {grpc::OK, "OK"}, 	
            {grpc::CANCELLED, "CANCELLED"}, 	
            {grpc::UNKNOWN, "UNKNOWN"}, 	
            {grpc::INVALID_ARGUMENT, "INVALID_ARGUMENT"}, 	
            {grpc::DEADLINE_EXCEEDED, "DEADLINE_EXCEEDED"}, 	
            {grpc::NOT_FOUND, "NOT_FOUND"}, 	
            {grpc::PERMISSION_DENIED, "PERMISSION_DENIED"}, 	
            {grpc::UNAUTHENTICATED, "UNAUTHENTICATED"}, 	
            {grpc::RESOURCE_EXHAUSTED, "RESOURCE_EXHAUSTED"}, 	
            {grpc::FAILED_PRECONDITION, "FAILED_PRECONDITION"}, 	
            {grpc::ABORTED, "ABORTED"}, 	
            {grpc::OUT_OF_RANGE, "OUT_OF_RANGE"}, 	
            {grpc::UNIMPLEMENTED, "UNIMPLEMENTED"}, 	
            {grpc::INTERNAL, "INTERNAL"}, 	
            {grpc::UNAVAILABLE, "UNAVAILABLE"}, 	
            {grpc::DATA_LOSS, "DATA_LOSS"}, 	
            {grpc::DO_NOT_USE, "DO_NOT_USE"}, 	
        };

        auto searchResult = codeMap.find(f_statusCode);
        if(searchResult != codeMap.end())
        {
            return searchResult->second;
        }
        else
        {
            return "";
        }
    }
}
