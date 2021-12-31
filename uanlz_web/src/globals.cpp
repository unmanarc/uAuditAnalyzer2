#include "globals.h"

using namespace UANLZ::WEB;

Mantids::Application::Logs::AppLog * Globals::applog = nullptr;
std::mutex Globals::mDatabase, Globals::mDirs;
boost::property_tree::ptree Globals::config_main;
Mantids::Application::Logs::RPCLog * Globals::rpclog = nullptr;
LoginRPCClientImpl Globals::loginRPCClient;

Mantids::RPC::Fast::FastRPC * Globals::fastRPC = nullptr;
Mantids::RPC::Web::WebServer * Globals::webServer = nullptr;;

Globals::Globals()
{
}

Mantids::Application::Logs::AppLog *Globals::getAppLog()
{
    return applog;
}

void Globals::setAppLog(Mantids::Application::Logs::AppLog *value)
{
    applog = value;
}

boost::property_tree::ptree *Globals::getConfig_main()
{
    return &config_main;
}

Mantids::RPC::Fast::FastRPC *Globals::getFastRPC()
{
    return fastRPC;
}

void Globals::setFastRPC(Mantids::RPC::Fast::FastRPC *value)
{
    fastRPC = value;
}

LoginRPCClientImpl *Globals::getLoginRPCClient()
{
    return &loginRPCClient;
}

Mantids::RPC::Web::WebServer *Globals::getWebServer()
{
    return webServer;
}

void Globals::setWebServer(Mantids::RPC::Web::WebServer *value)
{
    webServer = value;
}

Mantids::Application::Logs::RPCLog *Globals::getRPCLog()
{
    return rpclog;
}

void Globals::setRPCLog(Mantids::Application::Logs::RPCLog *value)
{
    rpclog = value;
}

