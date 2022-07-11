#include <mdz_prg_service/application.h>
#include <mdz_net_sockets/socket_tls.h>

#include "rpcserverimpl.h"
#include "webserverimpl.h"

#include "wsfunctions.h"
#include "globals.h"
#include "config.h"

#include <sys/types.h>

#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>

using namespace UANLZ::WEB;

using namespace Mantids::Application;

class Main : public Application
{
public:

    void _shutdown()
    {
    }

    int _start(int , char *[], Arguments::GlobalArguments *globalArguments)
    {
        std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

        // start program.
        LOG_APP->log(__func__, "","", Logs::LEVEL_INFO, 2048, "Starting... (Build date %s %s), PID: %" PRIi32,__DATE__, __TIME__, getpid());
        LOG_APP->log0(__func__,Logs::LEVEL_INFO, "Using config dir: %s", configDir.c_str());

        Globals::getLoginRPCClient()->start();

        if (!UANLZ::WEB::RPCServerImpl::createRPCListener())
        {
            _exit(-2);
        }

        if (!UANLZ::WEB::WebServerImpl::createWebServer())
        {
            _exit(-1);
        }

        LOG_APP->log0(__func__,Logs::LEVEL_INFO,  "Looking for static login resources...");

        auto staticContent = Globals::getLoginRPCClient()->getRemoteAuthManager()->getStaticContent();

        for ( unsigned int i=0; i<staticContent.size(); i++ )
        {
            auto jContent =  staticContent[i];

            if (jContent.isMember("path") && jContent.isMember("content"))
            {
                std::string path = JSON_ASSTRING(jContent,"path","");
                std::string content = JSON_ASSTRING(jContent,"content","");
                Globals::getWebServer()->addInternalContentElement(path,content);
                LOG_APP->log0(__func__,Logs::LEVEL_INFO,  "Adding web login static resource '%s'", path.c_str());
            }
            else {
                LOG_APP->log0(__func__,Logs::LEVEL_WARN,  "Warning: bad static resource (%d)", i);
            }
        }

        LOG_APP->log0(__func__,Logs::LEVEL_INFO,  (globalArguments->getDaemonName() + " initialized with PID: %d").c_str(), getpid());

        return 0;
    }

    void _initvars(int , char *[], Arguments::GlobalArguments * globalArguments)
    {
        // init variables (pre-config):
        globalArguments->setInifiniteWaitAtEnd(true);

        globalArguments->setLicense("GPLv3 (https://www.gnu.org/licenses/gpl-3.0.en.html)");
        globalArguments->setAuthor("Aarón Mizrachi");
        globalArguments->setEmail("aaron@unmanarc.com");

        globalArguments->setVersion(atoi(PROJECT_VER_MAJOR), atoi(PROJECT_VER_MINOR), atoi(PROJECT_VER_PATCH), "a");
        globalArguments->setDescription(PROJECT_DESCRIPTION);

        globalArguments->addCommandLineOption("Service Options", 'c', "config-dir" , "Configuration directory"  , "/etc/uauditanalyzer/" + globalArguments->getDaemonName(), Mantids::Memory::Abstract::Var::TYPE_STRING );
    }

    bool _config(int , char *argv[], Arguments::GlobalArguments * globalArguments)
    {
        Mantids::Network::Sockets::Socket_TLS::prepareTLS();

        // process config:
        unsigned int logMode = Logs::MODE_STANDARD;

        Mantids::Network::Sockets::Socket_TLS::prepareTLS();

        Logs::AppLog initLog( Logs::MODE_STANDARD);
        initLog.setPrintEmptyFields(true);
        initLog.setUsingAttributeName(false);
        initLog.setUserAlignSize(1);

        std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

        initLog.log0(__func__,Logs::LEVEL_INFO, "Loading configuration: %s", (configDir + "/config.ini").c_str());

        boost::property_tree::ptree config_main;

        if (access(configDir.c_str(),R_OK))
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Missing configuration dir: %s", configDir.c_str());
            return false;
        }

        chdir(configDir.c_str());

        if (!access("config.ini",R_OK))
            boost::property_tree::ini_parser::read_ini("config.ini",config_main);
        else
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Missing configuration: %s", "/config.ini");
            return false;
        }

        *(Globals::getConfig_main()) = config_main;

        if ( config_main.get<bool>("Logs.ToSyslog",false) ) logMode|=Logs::MODE_SYSLOG;
        Globals::setAppLog(new Logs::AppLog( logMode));
        LOG_APP->setPrintEmptyFields(true);
        LOG_APP->setUsingColors(config_main.get<bool>("Logs.ShowColors",true));
        LOG_APP->setUsingPrintDate(config_main.get<bool>("Logs.ShowDate",true));
        LOG_APP->setModuleAlignSize(26);
        LOG_APP->setUsingAttributeName(false);
        LOG_APP->setDebug(Globals::getConfig_main()->get<bool>("Logs.Debug",false));

        Globals::setRPCLog(new Logs::RPCLog(logMode));
        LOG_RPC->setPrintEmptyFields(true);
        LOG_RPC->setUsingColors(config_main.get<bool>("Logs.ShowColors",true));
        LOG_RPC->setUsingPrintDate(config_main.get<bool>("Logs.ShowDate",true));
        LOG_RPC->setDisableDomain(true);
        LOG_RPC->setDisableModule(true);
        LOG_RPC->setModuleAlignSize(26);
        LOG_RPC->setUsingAttributeName(false);
        LOG_RPC->setStandardLogSeparator(",");
        LOG_RPC->setDebug(config_main.get<bool>("Logs.Debug",false));

        // Initialize app vars here:
        Globals::getLoginRPCClient()->setApiKey(Globals::getConfig_main()->get<std::string>("LoginRPCClient.LoginApiKey","REPLACEME_XABCXAPIX_LOGIN"));
        Globals::getLoginRPCClient()->setAppName(Globals::getConfig_main()->get<std::string>("LoginRPCClient.AppName","UAUDITANLZ"));
        Globals::getLoginRPCClient()->setCaFile(Globals::getConfig_main()->get<std::string>("LoginRPCClient.CAFile","ca.crt"));
        Globals::getLoginRPCClient()->setRemoteHost(Globals::getConfig_main()->get<std::string>("LoginRPCClient.RemoteHost","127.0.0.1"));
        Globals::getLoginRPCClient()->setRemotePort(Globals::getConfig_main()->get<uint16_t>("LoginRPCClient.RemotePort",30301));
        Globals::getLoginRPCClient()->setUseIPv6(Globals::getConfig_main()->get<bool>("LoginRPCClient.ipv6",false));

        return true;
    }
};

int main(int argc, char *argv[])
{
    Main * main = new Main;
    return StartApplication(argc,argv,main);
}

