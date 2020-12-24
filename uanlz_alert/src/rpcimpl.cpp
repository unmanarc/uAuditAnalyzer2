#include "rpcimpl.h"

#include "globals.h"

#include "input/inputs.h"
#include "rules/rules.h"

#include <cx2_net_sockets/socket_tls.h>
#include <cx2_xrpc_fast/fastrpc.h>

using namespace CX2::Application;
using namespace UANLZ::JSONALERT;

RPCImpl::RPCImpl()
{
}

void RPCImpl::runRPClient()
{
    uint16_t remotePort = Globals::getConfig_main()->get<uint16_t>("RPCConnector.RemotePort",33001);
    std::string remoteAddr = Globals::getConfig_main()->get<std::string>("RPCConnector.RemoteAddr","127.0.0.1");
    std::string rpcApiKey = Globals::getConfig_main()->get<std::string>("RPCConnector.AlertsApiKey","REPLACEME_XABCXAPIX_ALERT");

    CX2::RPC::Fast::FastRPC fastRPC;

    fastRPC.addMethod( "control.RulesReload", {&controlRulesReload,nullptr} );
    fastRPC.addMethod( "stats.Rules",         {&statRules,nullptr} );
    fastRPC.addMethod( "stats.Inputs",        {&statInputs,nullptr} );

    for (;;)
    {
        CX2::Network::TLS::Socket_TLS sockRPCClient;

        if (!sockRPCClient.setTLSCertificateAuthorityPath( "rpcca.crt" ))
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Error starting RPC Connector to %s:%d: %s", remoteAddr.c_str(), remotePort, "Bad TLS Certificate Authority (rpcca.crt)");
            _exit(-3);
        }

        if ( sockRPCClient.connectTo( remoteAddr.c_str(), remotePort ) )
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,  "Connected to %s:%d", remoteAddr.c_str(), remotePort);
            sockRPCClient.writeString16(rpcApiKey);
            fastRPC.processConnection(&sockRPCClient,"SERVER");
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Error connecting to remote API Server @%s:%d: %s", remoteAddr.c_str(), remotePort, sockRPCClient.getLastError().c_str());
            sleep(3);
        }
    }
}

Json::Value RPCImpl::statInputs(void *, const Json::Value &)
{
    Json::Value j;
    j["ok"] = true;
    j["inputStats"] = Input::Inputs::getStats();
    return j;
}

Json::Value RPCImpl::statRules(void *, const Json::Value &)
{
    Json::Value j;
    j["ok"] = true;
    j["filterStats"] = Filters::Rules::getStats();
    return j;
}

Json::Value RPCImpl::controlRulesReload(void *, const Json::Value &)
{
    Json::Value j;
    j["ok"] = true;
    j["rules"] = Filters::Rules::reloadRules();
    j["actions"] = Filters::Rules::reloadActions();
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
