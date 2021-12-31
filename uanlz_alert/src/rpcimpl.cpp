#include "rpcimpl.h"

#include "globals.h"

#include "input/inputs.h"
#include "rules/rules.h"

#include <mdz_net_sockets/socket_tls.h>
#include <mdz_xrpc_fast/fastrpc.h>

using namespace Mantids::Application;
using namespace UANLZ::JSONALERT;

RPCImpl::RPCImpl()
{
}

void RPCImpl::runRPClient()
{
    uint16_t remotePort = Globals::getConfig_main()->get<uint16_t>("RPCConnector.RemotePort",33001);
    std::string remoteAddr = Globals::getConfig_main()->get<std::string>("RPCConnector.RemoteAddr","127.0.0.1");
    std::string rpcApiKey = Globals::getConfig_main()->get<std::string>("RPCConnector.AlertsApiKey","REPLACEME_XABCXAPIX_ALERT");

    Mantids::RPC::Fast::FastRPC fastRPC;

    // RULES:
    fastRPC.addMethod( "control.RulesReload",            {&controlRulesReload,nullptr} );
    fastRPC.addMethod( "control.RulesActions",           {&controlActionsReload,nullptr} );
    fastRPC.addMethod( "control.RemoveRule",             {&controlRemoveRule,nullptr} );
    fastRPC.addMethod( "control.SaveRunningRules",       {&controlSaveRunningRules,nullptr} );
    fastRPC.addMethod( "control.WriteRulesFileConfig",   {&controlWriteRulesFileConfig,nullptr} );
    fastRPC.addMethod( "stats.ReadRunningRules",         {&statReadRunningRules,nullptr} );
    fastRPC.addMethod( "stats.ReadRunningRule",          {&statReadRunningRule,nullptr} );
    fastRPC.addMethod( "stats.ReadRulesFileConfig",      {&statReadRulesFileConfig,nullptr} );
    fastRPC.addMethod( "stats.AreRulesModified",         {&statAreRulesModified,nullptr} );
    fastRPC.addMethod( "control.AddRule",                {&controlAddRule,nullptr} );
    fastRPC.addMethod( "control.EditRule",               {&controlEditRule,nullptr} );
    fastRPC.addMethod( "control.RuleUp",                 {&controlRuleUp,nullptr} );
    fastRPC.addMethod( "control.RuleDown",               {&controlRuleDown,nullptr} );

    // ACTIONS:
    fastRPC.addMethod( "control.WriteActionsFileConfig", {&controlWriteActionsFileConfig,nullptr} );
    fastRPC.addMethod( "control.AddAction",              {&controlAddAction,nullptr} );
    fastRPC.addMethod( "control.EditAction",             {&controlEditAction,nullptr} );
    fastRPC.addMethod( "control.RemoveAction",           {&controlRemoveAction,nullptr} );
    fastRPC.addMethod( "control.SaveRunningActions",     {&controlSaveRunningActions,nullptr} );
    fastRPC.addMethod( "stats.AreActionsModified",       {&statAreActionsModified,nullptr} );
    fastRPC.addMethod( "stats.ReadRunningActions",       {&statReadRunningActions,nullptr} );
    fastRPC.addMethod( "stats.ReadRunningAction",        {&statReadRunningAction,nullptr} );
    fastRPC.addMethod( "stats.ReadActionsFileConfig",    {&statReadActionsFileConfig,nullptr} );

    // INPUTS:
    fastRPC.addMethod( "stats.ReadRunningInputs",        {&statGetRunningInputs,nullptr} );
    fastRPC.addMethod( "stats.ReadInputsFileConfig",     {&statReadInputsFileConfig,nullptr} );
    fastRPC.addMethod( "control.AddInputToFileConfig",   {&AddInputToFileConfig,nullptr} );
    fastRPC.addMethod( "control.RemInputFromFileConfig",   {&RemInputFromFileConfig,nullptr} );


    fastRPC.addMethod( "stats.SystemStats",              {&statSystemStats,nullptr} );
    fastRPC.addMethod( "control.ReloadApp",              {&controlReloadApp,nullptr} );

    for (;;)
    {
        Mantids::Network::TLS::Socket_TLS sockRPCClient;

        if (!sockRPCClient.setTLSCertificateAuthorityPath( "rpcca.crt" ))
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Error starting RPC Connector to %s:%d: %s", remoteAddr.c_str(), remotePort, "Bad TLS Certificate Authority (rpcca.crt)");
            _exit(-3);
        }

        if ( sockRPCClient.connectTo( remoteAddr.c_str(), remotePort ) )
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,  "Connected to %s:%d", remoteAddr.c_str(), remotePort);
            sockRPCClient.writeStringEx<uint16_t>(rpcApiKey);
            fastRPC.processConnection(&sockRPCClient,"SERVER");
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Error connecting to remote API Server @%s:%d: %s", remoteAddr.c_str(), remotePort, sockRPCClient.getLastError().c_str());
            sleep(3);
        }
    }
}

json RPCImpl::controlWriteRulesFileConfig(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::writeRulesFileConfig(jConfig);
}

json RPCImpl::statReadRulesFileConfig(void *, const std::string &, const json &)
{
    return Filters::Rules::readRulesFileConfig();
}

json RPCImpl::statReadRunningRules(void *, const std::string &, const json &)
{
    return Filters::Rules::getCurrentRunningRules();
}

json RPCImpl::statReadRunningRule(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::getCurrentRunningRule(JSON_ASUINT(jConfig,"id",0xFFFFFFFF));
}

json RPCImpl::statAreRulesModified(void *, const std::string &, const json &)
{
    return Filters::Rules::getRulesModified();
}

json RPCImpl::controlRemoveRule(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::removeRule(JSON_ASUINT(jConfig,"pos",0xFFFFFFFF));
}

json RPCImpl::controlRemoveAction(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::removeAction(JSON_ASSTRING(jConfig,"name",""));
}

json RPCImpl::controlSaveRunningRules(void *, const std::string &, const json &)
{
    return Filters::Rules::writeRulesFileConfig(Filters::Rules::getCurrentRunningRules());
}

json RPCImpl::controlSaveRunningActions(void *, const std::string &, const json &)
{
    return Filters::Rules::writeActionsFileConfig(Filters::Rules::getCurrentRunningActions());
}

json RPCImpl::controlAddAction(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::addAction(jConfig);
}

json RPCImpl::controlEditAction(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::editAction(jConfig);
}

json RPCImpl::controlEditRule(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::editRule(JSON_ASUINT(jConfig,"pos",0xFFFFFFFF),jConfig["rule"]);
}

json RPCImpl::controlAddRule(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::addRule(JSON_ASUINT(jConfig,"pos",0xFFFFFFFF),jConfig["rule"]);
}

json RPCImpl::controlRuleUp(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::ruleUp(JSON_ASUINT(jConfig,"pos",0xFFFFFFFF));
}

json RPCImpl::controlRuleDown(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::ruleDown(JSON_ASUINT(jConfig,"pos",0xFFFFFFFF));
}

json RPCImpl::controlWriteActionsFileConfig(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::writeActionsFileConfig(jConfig);
}

json RPCImpl::statReadActionsFileConfig(void *, const std::string &, const json &)
{
    return Filters::Rules::readActionsFileConfig();
}

json RPCImpl::statAreActionsModified(void *, const std::string &, const json &)
{
    return Filters::Rules::getActionsModified();
}

json RPCImpl::statReadRunningActions(void *, const std::string &, const json &)
{
    return Filters::Rules::getCurrentRunningActions();
}

json RPCImpl::statReadRunningAction(void *, const std::string &, const json &jConfig)
{
    return Filters::Rules::getCurrentRunningAction(JSON_ASSTRING(jConfig,"name",""));
}

json RPCImpl::RemInputFromFileConfig(void *, const std::string &, const json &jConfig)
{
    return Input::Inputs::remInput(JSON_ASSTRING(jConfig,"id",""));
}

json RPCImpl::AddInputToFileConfig(void *, const std::string &, const json &jConfig)
{
    return Input::Inputs::addInput(jConfig);
}

json RPCImpl::statReadInputsFileConfig(void *, const std::string &, const json &)
{
    json j;
    j["modified"] = Input::Inputs::getModified();
    j["inputs"] = Input::Inputs::readInputs();
    return j;
}

json RPCImpl::statGetRunningInputs(void *, const std::string &, const json &)
{
    return Input::Inputs::getStats();
}

void sleepAndExit()
{
    sleep(2);
    _exit(0);
}

json RPCImpl::controlReloadApp(void *, const std::string &, const json &)
{
    json j = true;
    Globals::getAppLog()->log0(__func__,Logs::LEVEL_WARN,  "Reload requested via RPC");

    std::thread(sleepAndExit).detach();
    return j;
}

json RPCImpl::statSystemStats(void *,const std::string &, const json &)
{
    json j = Filters::Rules::getStats();
    return j;
}

json RPCImpl::controlRulesReload(void *,const std::string &, const json &)
{
    json j = Filters::Rules::reloadRules();
    return j;
}

json RPCImpl::controlActionsReload(void *,const std::string &, const json &)
{
    json j = Filters::Rules::reloadActions();
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
