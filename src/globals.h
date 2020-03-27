#ifndef GLOBALS_H
#define GLOBALS_H

#include <mutex>

// BOOST includes:
#include <boost/property_tree/ini_parser.hpp>

class Globals
{
public:
    Globals();

    static std::mutex * getDatabaseMutex();
    static void *getServerApp();
    static void setServerApp(void *value);

    static boost::property_tree::ptree * getConfig_main();

private:
    static void * serverApp;
    static std::mutex mDatabase;
    static boost::property_tree::ptree config_main;

};

#endif // GLOBALS_H
