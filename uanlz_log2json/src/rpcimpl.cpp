#include "rpcimpl.h"

#include "globals.h"

#include "output/outputs.h"
#include "input/inputs.h"
#include "events/events_manager.h"
#include "events/events_distributionthreads.h"

#include <cx2_net_sockets/socket_tls.h>
#include <cx2_xrpc_fast/fastrpc.h>

using namespace CX2::Application;
using namespace UANLZ::LOG2JSON;

RPCImpl::RPCImpl()
{

}

void RPCImpl::runRPClient()
{
    uint16_t remotePort = Globals::getConfig_main()->get<uint16_t>("RPCConnector.RemotePort",33001);
    std::string remoteAddr = Globals::getConfig_main()->get<std::string>("RPCConnector.RemoteAddr","127.0.0.1");
    std::string rpcApiKey = Globals::getConfig_main()->get<std::string>("RPCConnector.AlertsApiKey","REPLACEME_XABCXAPIX_LOG2JSON");

    CX2::RPC::Fast::FastRPC fastRPC;

    fastRPC.addMethod( "stats.EventsPerHost",              {&statEventsPerHost,nullptr} );
    fastRPC.addMethod( "stats.OutputsDistributionThreads", {&statOutputsDistributionThreads,nullptr} );
    fastRPC.addMethod( "stats.Outputs",                    {&statOutputs,nullptr} );
    fastRPC.addMethod( "stats.Inputs",                     {&statInputs,nullptr} );

    for (;;)
    {
        CX2::Network::TLS::Socket_TLS sockRPCClient;

        if (!sockRPCClient.setTLSCertificateAuthorityPath( "rpcca.crt" ))
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "Error starting RPC Connector to %s:%d: %s", remoteAddr.c_str(), remotePort, "Bad TLS Certificate Authority (rpcca.crt)");
            _exit(-3);
        }

        if ( sockRPCClient.connectTo( remoteAddr.c_str(), remotePort ) )
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,  "Connected to @%s:%d", remoteAddr.c_str(), remotePort);
            sockRPCClient.writeString16(rpcApiKey);
            fastRPC.processConnection(&sockRPCClient,"SERVER");
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "Error connecting to remote API Server @%s:%d: %s", remoteAddr.c_str(), remotePort, sockRPCClient.getLastError());
            sleep(3);
        }
    }
}

Json::Value RPCImpl::statEventsPerHost(void *, const Json::Value &)
{
    Json::Value j;
    j["hosts"] = AuditdEvents::Events_Manager::getJSONStats();
    j["ok"] = true;
    return j;
}

Json::Value RPCImpl::statOutputsDistributionThreads(void *, const Json::Value &)
{
    Json::Value j;
    j["events"] = AuditdEvents::Events_DistributionThreads::stats();
    j["ok"] = true;
    return j;
}

Json::Value RPCImpl::statOutputs(void *, const Json::Value &)
{
    Json::Value j;
    j["outputs"] = Output::Outputs::getStats();
    j["ok"] = true;
    return j;
}

Json::Value RPCImpl::statInputs(void *, const Json::Value &)
{
    Json::Value j;
    j["ok"] = true;
    j["inputs"] = Input::Inputs::getStats();
    return j;
}
/*
Json::Value RPCImpl::helloWorld(void *, const Json::Value &)
{
    Json::Value hw;
    hw["MSG"] = "Hello World!";
    return hw;
}
*/
