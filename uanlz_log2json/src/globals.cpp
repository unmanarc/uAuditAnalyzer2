#include "globals.h"

using namespace UANLZ::LOG2JSON;

Mantids::Application::Logs::AppLog * Globals::applog = nullptr;
std::mutex Globals::mDatabase, Globals::mDirs;
boost::property_tree::ptree Globals::config_main;

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


std::mutex *Globals::getDatabaseMutex()
{
    return &mDatabase;
}


boost::property_tree::ptree *Globals::getConfig_main()
{
    return &config_main;
}

