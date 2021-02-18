#include "webserverimpl.h"
#include "globals.h"
#include "defs.h"

#include <cx2_db_sqlite3/sqlconnector_sqlite3.h>
#include <cx2_auth_db/manager_db.h>
#include <cx2_xrpc_webserver/webserver.h>
#include <cx2_net_sockets/socket_tls.h>

#include  <boost/algorithm/string/predicate.hpp>

#include <sstream>
#include <ostream>

using namespace CX2::Application;
using namespace CX2::Authentication;
using namespace CX2::RPC::Web;
using namespace CX2::RPC;
using namespace CX2;
using namespace UANLZ::WEB;

#define UAUDITANLZ_APPNAME "UAUDITANLZ"

WebServerImpl::WebServerImpl()
{
}

Json::Value WebServerImpl::controlMethods(void *, Authentication::Manager *, Authentication::Session *, const Json::Value & jInput)
{
    std::string remoteMethod = jInput["remoteMethod"].asString();
    Json::Value j;
    if (!boost::starts_with(remoteMethod, "control.")) return j;
    j = Globals::getFastRPC()->runRemoteRPCMethod(jInput["target"].asString(), remoteMethod,jInput["payload"]);
    return j;
}

Json::Value WebServerImpl::statMethods(void *, Authentication::Manager *, Authentication::Session *, const Json::Value &jInput)
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
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Public Key");
        return false;
    }
    if (!sockWebListen->setTLSPrivateKeyPath( Globals::getConfig_main()->get<std::string>("WebServer.KeyFile","snakeoil.key").c_str()  ))
    {
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Private Key");
        return false;
    }

    if (sockWebListen->listenOn(listenPort ,listenAddr.c_str(), !Globals::getConfig_main()->get<bool>("WebServer.ipv6",false) ))
    {
        Authentication::Domains * authDomains = new Authentication::Domains;

        ////////////////////////////////////////////////////////////////
        /// Authentication DB
        ///
        Database::SQLConnector_SQLite3 * authDb = new Database::SQLConnector_SQLite3();
        //authDb->connect("/tmp/testdb.sqlite3");
        authDb->connectInMemory();
        Manager_DB * authManager = new Manager_DB(authDb);
        Secret passDataStats, passDataControl;

        if (!authManager->initScheme())
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Error (Driver: SQLite3), Unknown error during database scheme initialization.");
            return false;
        }

        // Create api account (stats):
        passDataStats.hash = Globals::getConfig_main()->get<std::string>("WebServer.StatsKey","stats");
        authManager->accountAdd("stats",passDataStats);
        // Create api account (control):
        passDataControl.hash = Globals::getConfig_main()->get<std::string>("WebServer.ControlKey","control");
        authManager->accountAdd("control",passDataControl);

        // Add application:
        if (!authManager->applicationAdd(UAUDITANLZ_APPNAME,"uAuditAnalyzer", "control"))
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Unexpected Error.");
            return false;
        }

        // Add users control+stats to our application
        authManager->applicationAccountAdd(UAUDITANLZ_APPNAME,"control");
        authManager->applicationAccountAdd(UAUDITANLZ_APPNAME,"stats");

        // Create attribute to applications:
        authManager->attribAdd({UAUDITANLZ_APPNAME,"control"},"Control Access");
        authManager->attribAdd({UAUDITANLZ_APPNAME,"stats"},"Stats Access");

        // Assign attributes to accounts:
        authManager->attribAccountAdd({UAUDITANLZ_APPNAME,"control"},"control"); // Add control attrib to control acct.
        authManager->attribAccountAdd({UAUDITANLZ_APPNAME,"stats"},"control"); // Add stats attrib to control acct.
        authManager->attribAccountAdd({UAUDITANLZ_APPNAME,"stats"},"stats");// Add stats attrib to stat acct.

        // Add the default domain / auth:
        authDomains->addDomain("",authManager);
        ////////////////////////////////////////////////////////////////

        MethodsManager *methodsManagers = new MethodsManager(UAUDITANLZ_APPNAME);
        // Add functions
        methodsManagers->addRPCMethod( "remote.stats",     {"stats"},   {&WebServerImpl::statMethods,nullptr} );
        methodsManagers->addRPCMethod( "remote.control",   {"control"}, {&WebServerImpl::controlMethods,nullptr} );

        WebServer * webServer = new WebServer;
        webServer->setRPCLog(Globals::getRPCLog());

        std::string resourcesPath = Globals::getConfig_main()->get<std::string>("WebServer.ResourcesPath","/var/www/uauditweb");
        if (!webServer->setResourcesLocalPath( resourcesPath ))
        {
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Error locating web server resources at %s",resourcesPath.c_str() );
            return false;
        }
        webServer->setAuthenticator(authDomains);
        webServer->setMethodManagers(methodsManagers);
        webServer->setSoftwareVersion(UANZL_VER_MAJOR, UANZL_VER_MINOR, UANZL_VER_SUBMINOR, UANZL_VER_CODENAME);
        webServer->setExtCallBackOnInitFailed(WebServerImpl::protoInitFail);

        webServer->acceptPoolThreaded(sockWebListen);

        Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,  "Status Web Server Listening @%s:%d", listenAddr.c_str(), listenPort);
        return true;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Status Web Server @%s:%d: %s", listenAddr.c_str(), listenPort, sockWebListen->getLastError().c_str());
        return false;
    }
}

bool WebServerImpl::protoInitFail(void * , Network::Streams::StreamSocket * sock, const char * remoteIP, bool )
{
    CX2::Network::TLS::Socket_TLS * secSocket = (CX2::Network::TLS::Socket_TLS *)sock;

    for (const auto & i :secSocket->getTLSErrorsAndClear())
    {
        if (!strstr(i.c_str(),"certificate unknown"))
            Globals::getAppLog()->log1(__func__, remoteIP,Logs::LEVEL_ERR, "TLS Protocol Initialization: %s", i.c_str());
    }
    return true;
}
