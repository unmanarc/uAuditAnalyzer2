#include "globals.h"

using namespace UANLZ::WEB;

CX2::Application::Logs::AppLog * Globals::applog = nullptr;
std::mutex Globals::mDatabase, Globals::mDirs;
boost::property_tree::ptree Globals::config_main;
CX2::Application::Logs::RPCLog * Globals::rpclog = nullptr;
LoginRPCClientImpl Globals::loginRPCClient;

CX2::RPC::Fast::FastRPC * Globals::fastRPC = nullptr;
CX2::RPC::Web::WebServer * Globals::webServer = nullptr;;

Globals::Globals()
{
}

CX2::Application::Logs::AppLog *Globals::getAppLog()
{
    return applog;
}

void Globals::setAppLog(CX2::Application::Logs::AppLog *value)
{
    applog = value;
}

boost::property_tree::ptree *Globals::getConfig_main()
{
    return &config_main;
}

CX2::RPC::Fast::FastRPC *Globals::getFastRPC()
{
    return fastRPC;
}

void Globals::setFastRPC(CX2::RPC::Fast::FastRPC *value)
{
    fastRPC = value;
}

LoginRPCClientImpl *Globals::getLoginRPCClient()
{
    return &loginRPCClient;
}

CX2::RPC::Web::WebServer *Globals::getWebServer()
{
    return webServer;
}

void Globals::setWebServer(CX2::RPC::Web::WebServer *value)
{
    webServer = value;
}

CX2::Application::Logs::RPCLog *Globals::getRPCLog()
{
    return rpclog;
}

void Globals::setRPCLog(CX2::Application::Logs::RPCLog *value)
{
    rpclog = value;
}

