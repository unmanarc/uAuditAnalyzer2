#include <cx2_prg_service/service.h>
#include <cx2_net_sockets/socket_tls.h>

#include "rpcserverimpl.h"
#include "webserverimpl.h"

#include "wsfunctions.h"
#include "globals.h"
#include "defs.h"

#include <sys/types.h>

#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>

using namespace UANLZ::WEB;

using namespace CX2::Application;

void _shutdown()
{
}

// TODO: create the config sample.

int _start(int , char *[], Arguments::GlobalArguments *globalArguments)
{
    std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

    // start program.
    Globals::getAppLog()->log(__func__, "","", Logs::LOG_LEVEL_INFO, 2048, "Starting... (Build date %s %s), PID: %u",__DATE__, __TIME__, getpid());
    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO, "Using config dir: %s", configDir.c_str());

    if (!UANLZ::WEB::RPCServerImpl::createRPCListener())
    {
        _exit(-2);
    }

    if (!UANLZ::WEB::WebServerImpl::createWebServer())
    {
        _exit(-1);
    }

    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,  (globalArguments->getDaemonName() + " initialized with PID: %d").c_str(), getpid());

    return 0;
}

void _initvars(int , char *[], Arguments::GlobalArguments * globalArguments)
{
    // init variables (pre-config):
    globalArguments->setInifiniteWaitAtEnd(true);

    globalArguments->setLicense("GPLv3 (https://www.gnu.org/licenses/gpl-3.0.en.html)");
    globalArguments->setAuthor("Aarón Mizrachi");
    globalArguments->setEmail("aaron@unmanarc.com");
    globalArguments->setVersion(UANZL_VER_MAJOR, UANZL_VER_MINOR, UANZL_VER_SUBMINOR, UANZL_VER_CODENAME);
    globalArguments->setDescription(std::string("Unmanarc's Auditd Analyzer Framework - Web Server"));

    globalArguments->addCommandLineOption("Service Options", 'c', "config-dir" , "Configuration directory"  , "/etc/uauditanalyzer/" + globalArguments->getDaemonName(), CX2::Memory::Vars::ABSTRACT_STRING );
}

bool _config(int , char *argv[], Arguments::GlobalArguments * globalArguments)
{
    // process config:
    unsigned int logMode = Logs::LOG_MODE_STANDARD;

    CX2::Network::TLS::Socket_TLS::prepareTLS();

    Logs::AppLog initLog(argv[0],"main", Logs::LOG_MODE_STANDARD);
    initLog.setPrintEmptyFields(true);
    initLog.setUsingAttributeName(false);
    initLog.setUserAlignSize(1);

    std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

    initLog.log0(__func__,Logs::LOG_LEVEL_INFO, "Loading configuration: %s", (configDir + "/config.ini").c_str());

    boost::property_tree::ptree config_main;

    if (access(configDir.c_str(),R_OK))
    {
        initLog.log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Missing configuration dir: %s", configDir.c_str());
        return false;
    }

    chdir(configDir.c_str());

    if (!access("config.ini",R_OK))
        boost::property_tree::ini_parser::read_ini("config.ini",config_main);
    else
    {
        initLog.log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Missing configuration: %s", "/config.ini");
        return false;
    }

    *(Globals::getConfig_main()) = config_main;

    if ( config_main.get<bool>("Logs.ToSyslog",true) ) logMode|=Logs::LOG_MODE_SYSLOG;
    Globals::setAppLog(new Logs::AppLog(argv[0], "main", logMode));
    Globals::getAppLog()->setPrintEmptyFields(true);
    Globals::getAppLog()->setUsingColors(config_main.get<bool>("Logs.ShowColors",true));
    Globals::getAppLog()->setUsingPrintDate(config_main.get<bool>("Logs.ShowDate",true));
    Globals::getAppLog()->setUserAlignSize(1);
    Globals::getAppLog()->setUsingAttributeName(false);
    Globals::getAppLog()->setDebug(Globals::getConfig_main()->get<bool>("Logs.Debug",false));

    return true;
}

