#ifndef GLOBALS_H
#define GLOBALS_H


#include <cx2_xrpc_fast/fastrpc.h>
#include <cx2_prg_logs/rpclog.h>

#include <boost/property_tree/ini_parser.hpp>
#include <cx2_prg_logs/applog.h>
#include <mutex>
#include <list>

namespace UANLZ { namespace WEB {

class Globals
{
public:
    Globals();

    static CX2::Application::Logs::AppLog *getAppLog();
    static void setAppLog(CX2::Application::Logs::AppLog *value);

    static boost::property_tree::ptree * getConfig_main();

    static std::string getRulesDir();
    static void setRulesDir(const std::string &value);

    static std::string getActionsDir();
    static void setActionsDir(const std::string &value);

    static CX2::RPC::Fast::FastRPC *getFastRPC();
    static void setFastRPC(CX2::RPC::Fast::FastRPC *value);
    static CX2::Application::Logs::RPCLog *getRPCLog();
    static void setRPCLog(CX2::Application::Logs::RPCLog *value);

private:
    static std::string rulesDir,actionsDir;
    static std::mutex mDatabase,mDirs;
    static boost::property_tree::ptree config_main;
    static CX2::Application::Logs::AppLog * applog;
    static CX2::Application::Logs::RPCLog * rpclog;

    static CX2::RPC::Fast::FastRPC * fastRPC;

};

}}

#endif // GLOBALS_H
