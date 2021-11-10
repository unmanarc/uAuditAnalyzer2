#include "globals.h"

using namespace UANLZ::LOG2JSON;

CX2::Application::Logs::AppLog * Globals::applog = nullptr;
std::mutex Globals::mDatabase, Globals::mDirs;
boost::property_tree::ptree Globals::config_main;

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


std::mutex *Globals::getDatabaseMutex()
{
    return &mDatabase;
}


boost::property_tree::ptree *Globals::getConfig_main()
{
    return &config_main;
}

