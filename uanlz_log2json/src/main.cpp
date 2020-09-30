#include <cx2_prg_service/service.h>
#include <cx2_auth_volatile/iauth_volatile.h>
#include <cx2_xrpc_webserver/webserver.h>
#include <cx2_net_sockets/socket_tcp.h>

#include "rpcimpl.h"
#include "globals.h"
#include "defs.h"

#include "vars/namedefs.h"

#include "events/events_distributionthreads.h"
#include "events/events_manager.h"
#include "output/outputs.h"
#include "input/inputs.h"

#include <sys/types.h>

#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>

using namespace UANLZ::LOG2JSON;

using namespace CX2::Application;
using namespace CX2::Authorization;
using namespace CX2::RPC::Web;
using namespace CX2::RPC;

void _shutdown()
{
}

int _start(int , char *[], Arguments::GlobalArguments *globalArguments)
{
    std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

    // start program.
    Globals::getAppLog()->log(__func__, "","", Logs::LOG_LEVEL_INFO, 2048, "Starting... (Build date %s %s), PID: %u",__DATE__, __TIME__, getpid());
    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO, "Using config dir: %s", configDir.c_str());

    // Start modules here...
    AuditdEvents::Events_Manager::startGC();

    AuditdEvents::Events_DistributionThreads::startDistributionThreads( Globals::getConfig_main()->get<size_t>("Processor.Threads",8) );
    AuditdEvents::Events_DistributionThreads::setQueueSize(Globals::getConfig_main()->get<size_t>("Processor.QueueSize",1000));

    // Start output threads...
    Output::Outputs::startThreads();

    // Start input threads...
    Input::Inputs::startThreads();

    std::thread(RPCImpl::runRPClient).detach();

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
    globalArguments->setDescription(std::string("Unmanarc's Auditd Analyzer Framework - Log to JSON Translator"));

    globalArguments->addCommandLineOption("Service Options", 'c', "config-dir" , "Configuration directory"  , "/etc/uauditanalyzer/" + globalArguments->getDaemonName(), CX2::Memory::Vars::ABSTRACT_STRING );
}

bool _config(int , char *argv[], Arguments::GlobalArguments * globalArguments)
{
    // process config:
    unsigned int logMode = Logs::LOG_MODE_STANDARD;

    AuditdEvents::NameDefs::init();

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

    if (!access((configDir + "/config.ini").c_str(),R_OK))
        boost::property_tree::ini_parser::read_ini( (configDir + "/config.ini").c_str(),config_main);
    else
    {
        initLog.log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Missing configuration: %s", (configDir + "/config.ini").c_str());
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string dirPath = configDir + "/receptors.d";
    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            std::set<std::string> files;
            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                    files.insert(ent->d_name);
            }
            closedir (dir);

            for (const std::string & file: files)
            {
                boost::property_tree::ptree filters;
                std::string fullFilePath = dirPath + "/" + file;
                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO, "Loading Log Receptor configuration from file: %s", fullFilePath.c_str());
                Input::Inputs::loadConfig(fullFilePath);
            }
        }
        else
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "Failed to list directory: %s, no receptors loaded.", dirPath.c_str());
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Missing/Unreadable receptors directory: %s", dirPath.c_str());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    dirPath = configDir + "/dispatchers.d";
    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            std::set<std::string> files;
            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                    files.insert(ent->d_name);
            }
            closedir (dir);

            for (const std::string & file: files)
            {
                boost::property_tree::ptree filters;
                std::string fullFilePath = dirPath + "/" + file;
                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO, "Loading TCP JSON Dispatcher configuration from file: %s", fullFilePath.c_str());
                Output::Outputs::loadConfig(fullFilePath);
            }
        }
        else
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "Failed to list directory: %s, no TCP JSON dispatchers loaded.", dirPath.c_str());
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Missing/Unreadable TCP JSON dispatchers directory: %s", dirPath.c_str());
        return false;
    }

    return true;
}
