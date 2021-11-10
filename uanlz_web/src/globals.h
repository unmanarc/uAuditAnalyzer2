#ifndef GLOBALS_H
#define GLOBALS_H


#include "loginrpcclientimpl.h"
#include <cx2_xrpc_fast/fastrpc.h>
#include <cx2_prg_logs/rpclog.h>
#include <cx2_xrpc_webserver/webserver.h>

#include <boost/property_tree/ini_parser.hpp>
#include <cx2_prg_logs/applog.h>
#include <mutex>
#include <list>



namespace UANLZ { namespace WEB {

class Globals
{
public:
    Globals();

    // Main Log:
    static CX2::Application::Logs::AppLog *getAppLog();
    static void setAppLog(CX2::Application::Logs::AppLog *value);

    // RPC Log:
    static CX2::Application::Logs::RPCLog *getRPCLog();
    static void setRPCLog(CX2::Application::Logs::RPCLog *value);

    // Main Config:
    static boost::property_tree::ptree * getConfig_main();

    // Inter-Process Fast RPC Engine:
    static CX2::RPC::Fast::FastRPC *getFastRPC();
    static void setFastRPC(CX2::RPC::Fast::FastRPC *value);

    static LoginRPCClientImpl * getLoginRPCClient();

    // Web Server:
    static CX2::RPC::Web::WebServer *getWebServer();
    static void setWebServer(CX2::RPC::Web::WebServer *value);

private:
    static std::mutex mDatabase,mDirs;
    static boost::property_tree::ptree config_main;
    static CX2::Application::Logs::AppLog * applog;
    static CX2::Application::Logs::RPCLog * rpclog;

    static CX2::RPC::Web::WebServer * webServer;
    static CX2::RPC::Fast::FastRPC * fastRPC;
    static LoginRPCClientImpl loginRPCClient;
};

}}

#endif // GLOBALS_H
