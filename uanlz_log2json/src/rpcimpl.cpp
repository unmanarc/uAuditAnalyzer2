#include "rpcimpl.h"

#include "globals.h"

#include "output/outputs.h"
#include "input/inputs.h"
#include "events/events_manager.h"
#include "events/events_distributionthreads.h"

#include <mdz_net_sockets/socket_tls.h>
#include <mdz_xrpc_fast/fastrpc.h>
#include <inttypes.h>

using namespace Mantids::Application;
using namespace UANLZ::LOG2JSON;

RPCImpl::RPCImpl()
{

}

void RPCImpl::runRPClient()
{
    uint16_t remotePort = Globals::getConfig_main()->get<uint16_t>("RPCConnector.RemotePort",33001);
    std::string remoteAddr = Globals::getConfig_main()->get<std::string>("RPCConnector.RemoteAddr","127.0.0.1");
    std::string rpcApiKey = Globals::getConfig_main()->get<std::string>("RPCConnector.AlertsApiKey","REPLACEME_XABCXAPIX_LOG2JSON");

    Mantids::RPC::Fast::FastRPC fastRPC;

    fastRPC.addMethod( "stats.EventsPerHost",              {&statEventsPerHost,nullptr} );
    fastRPC.addMethod( "stats.OutputsDistributionThreads", {&statOutputsDistributionThreads,nullptr} );
    fastRPC.addMethod( "stats.Outputs",                    {&statOutputs,nullptr} );
    fastRPC.addMethod( "stats.Inputs",                     {&statInputs,nullptr} );

    for (;;)
    {
        Mantids::Network::Sockets::Socket_TLS sockRPCClient;

        // Set the SO default security level:
        sockRPCClient.keys.setSecurityLevel(-1);

        if (!sockRPCClient.keys.loadCAFromPEMFile( "rpcca.crt" ))
        {
            LOG_APP->log0(__func__,Logs::LEVEL_ERR, "Error starting RPC Connector to %s:%" PRIu16 ": %s", remoteAddr.c_str(), remotePort, "Bad TLS Certificate Authority (rpcca.crt)");
            _exit(-3);
        }

        if ( sockRPCClient.connectTo( remoteAddr.c_str(), remotePort ) )
        {
            LOG_APP->log0(__func__,Logs::LEVEL_INFO,  "Connected to @%s:%" PRIu16, remoteAddr.c_str(), remotePort);
            sockRPCClient.writeStringEx<uint16_t>(rpcApiKey);
            fastRPC.processConnection(&sockRPCClient,"SERVER");
        }
        else
        {
            LOG_APP->log0(__func__,Logs::LEVEL_ERR, "Error connecting to remote API Server @%s:%" PRIu16 ": %s", remoteAddr.c_str(), remotePort, sockRPCClient.getLastError().c_str());
            sleep(3);
        }
    }
}

json RPCImpl::statEventsPerHost(void *,const std::string &, const json &, void*, const std::string &)
{
    json j;
    j["hosts"] = AuditdEvents::Events_Manager::getJSONStats();
    j["ok"] = true;
    return j;
}

json RPCImpl::statOutputsDistributionThreads(void *,const std::string &, const json &, void*, const std::string &)
{
    json j;
    j["events"] = AuditdEvents::Events_DistributionThreads::stats();
    j["ok"] = true;
    return j;
}

json RPCImpl::statOutputs(void *,const std::string &, const json &, void*, const std::string &)
{
    json j;
    j["outputs"] = Output::Outputs::getStats();
    j["ok"] = true;
    return j;
}

json RPCImpl::statInputs(void *,const std::string &, const json &, void*, const std::string &)
{
    json j;
    j["ok"] = true;
    j["inputs"] = Input::Inputs::getStats();
    return j;
}
/*
json RPCImpl::helloWorld(void *, const json &)
{
    json hw;
    hw["MSG"] = "Hello World!";
    return hw;
}
*/
