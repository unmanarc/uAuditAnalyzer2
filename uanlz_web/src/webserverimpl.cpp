#include "webserverimpl.h"
#include "globals.h"
#include "config.h"

#include <inttypes.h>

#include <mdz_net_sockets/socket_tls.h>
#include <boost/algorithm/string/predicate.hpp>

#include <memory>
#include <sstream>
#include <ostream>
#include <fstream>

using namespace Mantids::Application;
using namespace Mantids::Authentication;
using namespace Mantids::RPC::Web;
using namespace Mantids::RPC;
using namespace Mantids;
using namespace UANLZ::WEB;

WebServerImpl::WebServerImpl()
{
}

json WebServerImpl::rmtCaller(const std::string &caller, Authentication::Manager * auth, Authentication::Session * sess, const json &jInput)
{
    std::string remoteMethod = JSON_ASSTRING(jInput,"remoteMethod","");
    json j,error;
    if (!boost::starts_with(remoteMethod, caller)) return j;
    j = Globals::getFastRPC()->runRemoteRPCMethod(JSON_ASSTRING(jInput,"target",""), remoteMethod,jInput["payload"],&error);

    if (JSON_ASBOOL(error,"succeed",false) == false)
    {
        LOG_APP->log2(__func__, sess->getUserID(),"", Logs::LEVEL_ERR, "Error (%d) Executing Method '%s' at '%s': '%s'",
                                   JSON_ASINT(error,"errorId",-1),
                                   remoteMethod.c_str(),
                                   JSON_ASCSTRING(jInput,"target",""),
                                   JSON_ASCSTRING(error,"errorMessage","")
                                   );
    }
    else
    {
        LOG_APP->log2(__func__, sess->getUserID(),"", Logs::LEVEL_INFO, "Method '%s' Executed at '%s'.",
                                   remoteMethod.c_str(),
                                   JSON_ASCSTRING(jInput,"target","")
                                   );
    }
    return j;
}

json WebServerImpl::controlMethods(void *, Authentication::Manager *auth, Authentication::Session * sess, const json & jInput)
{
    return rmtCaller( "control.", auth,sess,jInput );
}

json WebServerImpl::statMethods(void *, Authentication::Manager * auth, Authentication::Session * sess, const json &jInput)
{
  return rmtCaller( "stats.", auth,sess,jInput );
}

bool WebServerImpl::createWebServer()
{
    std::string sAppName = Globals::getConfig_main()->get<std::string>("LoginRPCClient.AppName","UAUDITANLZ");
    Mantids::Network::Sockets::Socket_TLS * sockWebListen = new Mantids::Network::Sockets::Socket_TLS();

    uint16_t listenPort = Globals::getConfig_main()->get<uint16_t>("WebServer.ListenPort",33000);
    std::string listenAddr = Globals::getConfig_main()->get<std::string>("WebServer.ListenAddr","0.0.0.0");

    // Set the SO default security level:
    sockWebListen->keys.setSecurityLevel(-1);

    if (!sockWebListen->keys.loadPublicKeyFromPEMFile(  Globals::getConfig_main()->get<std::string>("WebServer.CertFile","snakeoil.crt").c_str()  ))
    {
        LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Web Server @%s:%" PRIu16 ": %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Public Key");
        return false;
    }
    if (!sockWebListen->keys.loadPrivateKeyFromPEMFile( Globals::getConfig_main()->get<std::string>("WebServer.KeyFile","snakeoil.key").c_str()  ))
    {
        LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Web Server @%s:%" PRIu16 ": %s", listenAddr.c_str(), listenPort, "Bad TLS WEB Server Private Key");
        return false;
    }

    if (sockWebListen->listenOn(listenPort ,listenAddr.c_str(), !Globals::getConfig_main()->get<bool>("WebServer.ipv6",false) ))
    {
        Authentication::Domains * authDomains = new Authentication::Domains;

        ////////////////////////////////////////////////////////////////
        /// Authentication DB
        ///
        auto * authManager = Globals::getLoginRPCClient()->getRemoteAuthManager();
        Secret passDataStats, passDataControl;

        // Create attribute to applications:]
        if (!authManager->attribExist({sAppName,"control"}))
        {
            if (!authManager->attribAdd({sAppName,"control"},"Control Access"))
            {
                LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to create attribute in the authentication server.");
                return false;
            }
            else
                LOG_APP->log0(__func__,Logs::LEVEL_INFO, "Attrib 'control' initialized.");

        }
        if (!authManager->attribExist({sAppName,"stats"}))
        {
            if (!authManager->attribAdd({sAppName,"stats"},"Stats Access"))
            {
                LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Failed to create attribute in the authentication server.");
                return false;
            }
            else
                LOG_APP->log0(__func__,Logs::LEVEL_INFO, "Attrib 'stats' initialized.");
        }
        // Add the default domain / auth:
        authDomains->addDomain("",authManager);
        ////////////////////////////////////////////////////////////////

        MethodsManager *methodsManagers = new MethodsManager(sAppName);
        // Add functions
        methodsManagers->addRPCMethod( "remote.stats",     {"stats"},   {&WebServerImpl::statMethods,nullptr} );
        methodsManagers->addRPCMethod( "remote.control",   {"control"}, {&WebServerImpl::controlMethods,nullptr} );

        WebServer * webServer = new WebServer;
        Globals::setWebServer(webServer);
        webServer->setRPCLog(LOG_RPC);

        std::string resourcesPath = Globals::getConfig_main()->get<std::string>("WebServer.ResourcesPath","/var/www/uauditweb");
        if (!webServer->setDocumentRootPath( resourcesPath ))
        {
            LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Error locating web server resources at %s",resourcesPath.c_str() );
            return false;
        }
        webServer->setAuthenticator(authDomains);
        webServer->setMethodManagers(methodsManagers);
        webServer->setSoftwareVersion(atoi(PROJECT_VER_MAJOR), atoi(PROJECT_VER_MINOR), atoi(PROJECT_VER_PATCH), "a");
        webServer->setExtCallBackOnInitFailed(WebServerImpl::protoInitFail);

        webServer->acceptPoolThreaded(sockWebListen);

        LOG_APP->log0(__func__,Logs::LEVEL_INFO,  "Status Web Server Listening @%s:%" PRIu16, listenAddr.c_str(), listenPort);
        return true;
    }
    else
    {
        LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Error starting Status Web Server @%s:%" PRIu16 ": %s", listenAddr.c_str(), listenPort, sockWebListen->getLastError().c_str());
        delete sockWebListen;
        return false;
    }
}

bool WebServerImpl::protoInitFail(void * , Network::Sockets::Socket_StreamBase * sock, const char * remoteIP, bool )
{
    Mantids::Network::Sockets::Socket_TLS * secSocket = (Mantids::Network::Sockets::Socket_TLS *)sock;

    for (const auto & i :secSocket->getTLSErrorsAndClear())
    {
        if (!strstr(i.c_str(),"certificate unknown"))
            LOG_APP->log1(__func__, remoteIP,Logs::LEVEL_ERR, "TLS Protocol Initialization: %s", i.c_str());
    }
    return true;
}
