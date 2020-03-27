#include "globals.h"

std::mutex Globals::mDatabase;
void * Globals::serverApp = nullptr;
boost::property_tree::ptree Globals::config_main;

Globals::Globals()
{

}

std::mutex *Globals::getDatabaseMutex()
{
    return &mDatabase;
}

void *Globals::getServerApp()
{
    return serverApp;
}

void Globals::setServerApp(void *value)
{
    serverApp = value;
}

boost::property_tree::ptree *Globals::getConfig_main()
{
    return &config_main;
}
