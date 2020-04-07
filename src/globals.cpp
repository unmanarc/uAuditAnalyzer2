#include "globals.h"

std::mutex Globals::mDatabase;
void * Globals::serverApp = nullptr;
boost::property_tree::ptree Globals::config_main;
std::list<Output_Base *> Globals::outputBases;

Globals::Globals()
{
}

void Globals::pushToOutputBases(const Json::Value &eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId)
{
    for (Output_Base * ob : outputBases)
    {
        ob->logAuditEvent(eventJSON,eventId);
    }
}

void Globals::addOutputBaseAndStartThreads(Output_Base *ob)
{
    outputBases.push_back(ob);
    ob->startThread();
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
