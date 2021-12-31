#ifndef GLOBALS_H
#define GLOBALS_H


#include "loginrpcclientimpl.h"
#include <mdz_xrpc_fast/fastrpc.h>
#include <mdz_prg_logs/rpclog.h>
#include <mdz_xrpc_webserver/webserver.h>

#include <boost/property_tree/ini_parser.hpp>
#include <mdz_prg_logs/applog.h>
#include <mutex>
#include <list>



namespace UANLZ { namespace WEB {

class Globals
{
public:
    Globals();

    // Main Log:
    static Mantids::Application::Logs::AppLog *getAppLog();
    static void setAppLog(Mantids::Application::Logs::AppLog *value);

    // RPC Log:
    static Mantids::Application::Logs::RPCLog *getRPCLog();
    static void setRPCLog(Mantids::Application::Logs::RPCLog *value);

    // Main Config:
    static boost::property_tree::ptree * getConfig_main();

    // Inter-Process Fast RPC Engine:
    static Mantids::RPC::Fast::FastRPC *getFastRPC();
    static void setFastRPC(Mantids::RPC::Fast::FastRPC *value);

    static LoginRPCClientImpl * getLoginRPCClient();

    // Web Server:
    static Mantids::RPC::Web::WebServer *getWebServer();
    static void setWebServer(Mantids::RPC::Web::WebServer *value);

private:
    static std::mutex mDatabase,mDirs;
    static boost::property_tree::ptree config_main;
    static Mantids::Application::Logs::AppLog * applog;
    static Mantids::Application::Logs::RPCLog * rpclog;

    static Mantids::RPC::Web::WebServer * webServer;
    static Mantids::RPC::Fast::FastRPC * fastRPC;
    static LoginRPCClientImpl loginRPCClient;
};

}}

#endif // GLOBALS_H
