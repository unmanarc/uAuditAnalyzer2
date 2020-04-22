#ifndef GLOBALS_H
#define GLOBALS_H

#include <boost/property_tree/ini_parser.hpp>
#include <mutex>
#include <list>

#include "output/output_base.h"

class Globals
{
public:
    Globals();

    static void pushToOutputBases(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId);
    static void addOutputBaseAndStartThreads(Output_Base *);

    static void writeStatsOnOutputBases(std::string outputDir);

    static std::mutex * getDatabaseMutex();
    static void *getServerApp();
    static void setServerApp(void *value);

    static boost::property_tree::ptree * getConfig_main();

    static std::string getRulesDir();
    static void setRulesDir(const std::string &value);

    static std::string getActionsDir();
    static void setActionsDir(const std::string &value);

private:
    static void * serverApp;
    static std::string rulesDir,actionsDir;
    static std::mutex mDatabase,mDirs;
    static boost::property_tree::ptree config_main;
    static std::list<Output_Base *> outputBases;

};

#endif // GLOBALS_H
