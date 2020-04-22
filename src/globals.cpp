#include "globals.h"


std::string Globals::rulesDir,Globals::actionsDir;
std::mutex Globals::mDatabase, Globals::mDirs;
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

void Globals::writeStatsOnOutputBases(std::string outputDir)
{
    for (Output_Base * ob : outputBases)
    {
        ob->writeStats(outputDir);
    }
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
