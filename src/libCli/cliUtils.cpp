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
}
