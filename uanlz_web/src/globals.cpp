#include "globals.h"

using namespace UANLZ::WEB;

CX2::Application::Logs::AppLog * Globals::applog = nullptr;
std::string Globals::rulesDir,Globals::actionsDir;
std::mutex Globals::mDatabase, Globals::mDirs;
boost::property_tree::ptree Globals::config_main;
CX2::Application::Logs::RPCLog * Globals::rpclog = nullptr;

CX2::RPC::Fast::FastRPC * Globals::fastRPC = nullptr;

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

std::string Globals::getRulesDir()
{
    std::string r;
    mDirs.lock();
    r = rulesDir;
    mDirs.unlock();
    return r;
}

void Globals::setRulesDir(const std::string &value)
{
    mDirs.lock();
    rulesDir = value;
    mDirs.unlock();
}

std::string Globals::getActionsDir()
{
    std::string r;
    mDirs.lock();
    r = actionsDir;
    mDirs.unlock();
    return r;
}

void Globals::setActionsDir(const std::string &value)
{
    mDirs.lock();
    actionsDir = value;
    mDirs.unlock();
}

CX2::RPC::Fast::FastRPC *Globals::getFastRPC()
{
    return fastRPC;
}

void Globals::setFastRPC(CX2::RPC::Fast::FastRPC *value)
{
    fastRPC = value;
}

CX2::Application::Logs::RPCLog *Globals::getRPCLog()
{
    return rpclog;
}

void Globals::setRPCLog(CX2::Application::Logs::RPCLog *value)
{
    rpclog = value;
}

