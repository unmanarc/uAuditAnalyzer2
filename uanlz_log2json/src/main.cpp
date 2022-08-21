#include <mdz_prg_service/application.h>
#include <mdz_xrpc_webserver/webserver.h>
#include <mdz_net_sockets/socket_tcp.h>
#include <mdz_net_sockets/socket_tls.h>

#include "rpcimpl.h"
#include "globals.h"
#include "config.h"

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

#include <inttypes.h>
#include <sys/stat.h>

using namespace UANLZ::LOG2JSON;

using namespace Mantids::Application;
using namespace Mantids::Authentication;
using namespace Mantids::RPC::Web;
using namespace Mantids::RPC;

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

        // Start modules here...
        AuditdEvents::Events_Manager::startGC();

        AuditdEvents::Events_DistributionThreads::startDistributionThreads( Globals::getConfig_main()->get<size_t>("Processor.Threads",8) );
        AuditdEvents::Events_DistributionThreads::setQueueSize(Globals::getConfig_main()->get<size_t>("Processor.QueueSize",1000));

        // Start output threads...
        Output::Outputs::startThreads();

        // Start input threads...
        Input::Inputs::startThreads();

        std::thread(RPCImpl::runRPClient).detach();

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

        AuditdEvents::NameDefs::init();

        Logs::AppLog initLog(Logs::MODE_STANDARD);
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

        if (!access((configDir + "/config.ini").c_str(),R_OK))
        {
            struct stat stats;
            // Check file properties...
            stat((configDir + "/config.ini").c_str(), &stats);

            int smode = stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if ( smode != 0600 )
            {
                initLog.log0(__func__,Logs::LEVEL_WARN, "config.ini file permissions are not 0600 and API key may be exposed, changing...");

                if (chmod((configDir + "/config.ini").c_str(),0600))
                {
                    initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Configuration file permissions can't be changed to 0600");
                    return false;
                }
                initLog.log0(__func__,Logs::LEVEL_INFO, "config.ini file permissions changed to 0600");
            }

            boost::property_tree::ini_parser::read_ini( (configDir + "/config.ini").c_str(),config_main);
        }
        else
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Missing configuration: %s", (configDir + "/config.ini").c_str());
            return false;
        }

        *(Globals::getConfig_main()) = config_main;

        if ( config_main.get<bool>("Logs.ToSyslog",true) ) logMode|=Logs::MODE_SYSLOG;
        Globals::setAppLog(new Logs::AppLog(logMode));
        LOG_APP->setPrintEmptyFields(true);
        LOG_APP->setUsingColors(config_main.get<bool>("Logs.ShowColors",true));
        LOG_APP->setUsingPrintDate(config_main.get<bool>("Logs.ShowDate",true));
        LOG_APP->setUserAlignSize(1);
        LOG_APP->setUsingAttributeName(false);
        LOG_APP->setDebug(Globals::getConfig_main()->get<bool>("Logs.Debug",false));

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
                    LOG_APP->log0(__func__,Logs::LEVEL_INFO, "Loading Log Receptor configuration from file: %s", fullFilePath.c_str());
                    Input::Inputs::loadConfig(fullFilePath);
                }
            }
            else
                LOG_APP->log0(__func__,Logs::LEVEL_ERR, "Failed to list directory: %s, no receptors loaded.", dirPath.c_str());
        }
        else
        {
            LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Missing/Unreadable receptors directory: %s", dirPath.c_str());
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
                    LOG_APP->log0(__func__,Logs::LEVEL_INFO, "Loading TCP JSON Dispatcher configuration from file: %s", fullFilePath.c_str());
                    Output::Outputs::loadConfig(fullFilePath);
                }
            }
            else
                LOG_APP->log0(__func__,Logs::LEVEL_ERR, "Failed to list directory: %s, no TCP JSON dispatchers loaded.", dirPath.c_str());
        }
        else
        {
            LOG_APP->log0(__func__,Logs::LEVEL_CRITICAL, "Missing/Unreadable TCP JSON dispatchers directory: %s", dirPath.c_str());
            return false;
        }

        return true;
    }

};

int main(int argc, char *argv[])
{
    Main * main = new Main;
    return StartApplication(argc,argv,main);
}
