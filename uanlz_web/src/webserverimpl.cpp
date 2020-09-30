#include "webserverimpl.h"
#include "globals.h"
#include "defs.h"


#include <cx2_auth_volatile/iauth_volatile.h>
#include <cx2_xrpc_webserver/webserver.h>
#include <cx2_net_sockets/socket_tls.h>

#include  <boost/algorithm/string/predicate.hpp>

#include <sstream>
#include <ostream>

using namespace CX2::Application;
using namespace CX2::Authorization;
using namespace CX2::RPC::Web;
using namespace CX2::RPC;
using namespace CX2;
using namespace UANLZ::WEB;

WebServerImpl::WebServerImpl()
{
}

Json::Value WebServerImpl::controlMethods(void *, IAuth *, Session::IAuth_Session *, const Json::Value & jInput)
{
    std::string remoteMethod = jInput["remoteMethod"].asString();
    Json::Value j;
    if (!boost::starts_with(remoteMethod, "control.")) return j;
    j = Globals::getFastRPC()->runRemoteRPCMethod(jInput["target"].asString(), remoteMethod,jInput["payload"]);
    return j;
}

Json::Value WebServerImpl::statMethods(void *, IAuth *, Session::IAuth_Session *, const Json::Value &jInput)
{
    std::string remoteMethod = jInput["remoteMethod"].asString();
    Json::Value j;
    if (!boost::starts_with(remoteMethod, "stats.")) return j;
    j = Globals::getFastRPC()->runRemoteRPCMethod(jInput["target"].asString(), remoteMethod,jInput["payload"]);
    return j;
}

bool WebServerImpl::createWebServer()
{
    CX2::Network::TLS::Socket_TLS * sockWebListen = new CX2::Network::TLS::Socket_TLS;

    uint16_t listenPort = Globals::getConfig_main()->get<uint16_t>("WebServer.ListenPort",33000);
    std::string listenAddr = Globals::getConfig_main()->get<std::string>("WebServer.ListenAddr","0.0.0.0");

    if (!sockWebListen->setTLSPublicKeyPath(  Globals::getConfig_main()->get<std::string>("WebServer.CertFile","snakeoil.crt").c_str()  ))
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Public Key");
        return false;
    }
    if (!sockWebListen->setTLSPrivateKeyPath( Globals::getConfig_main()->get<std::string>("WebServer.KeyFile","snakeoil.key").c_str()  ))
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Private Key");
        return false;
    }

    if (sockWebListen->listenOn(listenPort ,listenAddr.c_str(), !Globals::getConfig_main()->get<bool>("WebServer.ipv6",false) ))
    {
        IAuth_Domains * authDomains = new IAuth_Domains;
        MethodsManager *methodsManagers = new MethodsManager;
        IAuth_Volatile * auth = new IAuth_Volatile;
        DataStructs::sPasswordData passDataStats, passDataControl;

        // Create api account (stats):
        passDataStats.hash = Globals::getConfig_main()->get<std::string>("WebServer.StatsKey","stats");
        auth->accountAdd("stats",passDataStats);
        auth->attribAdd("stats","Stats Access");
        auth->attribAccountAdd("stats","stats");

        // Create api account (control):
        passDataControl.hash = Globals::getConfig_main()->get<std::string>("WebServer.ControlKey","control");
        auth->accountAdd("control",passDataControl);
        auth->attribAdd("control","Control Access");
        auth->attribAccountAdd("control","control"); // Add control to control.
        auth->attribAccountAdd("stats","control"); // Add stats to control.

        // Add functions

        methodsManagers->addRPCMethod( "remote.stats",     {"stats"},   {&WebServerImpl::statMethods,nullptr} );
        methodsManagers->addRPCMethod( "remote.control",   {"control"}, {&WebServerImpl::controlMethods,nullptr} );

        // Add the default domain / auth:
        authDomains->addDomain("",auth);

        WebServer * webServer = new WebServer;
        std::string resourcesPath = Globals::getConfig_main()->get<std::string>("WebServer.ResourcesPath","/var/www/uauditweb");
        if (!webServer->setResourcesLocalPath( resourcesPath ))
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error locating web server resources at %s",resourcesPath.c_str() );
            return false;
        }
        webServer->setAuthenticator(authDomains);
        webServer->setMethodManagers(methodsManagers);
        webServer->setSoftwareVersion(UANZL_VER_MAJOR, UANZL_VER_MINOR, UANZL_VER_SUBMINOR, UANZL_VER_CODENAME);
        webServer->setExtCallBackOnInitFailed(WebServerImpl::protoInitFail);

        webServer->acceptPoolThreaded(sockWebListen);

        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,  "Status Web Server Listening @%s:%d", listenAddr.c_str(), listenPort);
        return true;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting Status Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, sockWebListen->getLastError());
        return false;
    }
}

bool WebServerImpl::protoInitFail(void * , Network::Streams::StreamSocket * sock, const char * remoteIP, bool )
{
    CX2::Network::TLS::Socket_TLS * secSocket = (CX2::Network::TLS::Socket_TLS *)sock;

    for (const auto & i :secSocket->getTLSErrorsAndClear())
    {
        Globals::getAppLog()->log1(__func__, remoteIP,Logs::LOG_LEVEL_ERR, "TLS Protocol Initialization: %s", i.c_str());
    }
    return true;
}
