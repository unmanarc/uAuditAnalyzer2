#include <mdz_prg_service/application.h>
#include <mdz_xrpc_webserver/webserver.h>
#include <mdz_net_sockets/socket_tcp.h>
#include <mdz_net_sockets/socket_tls.h>

#include "rpcimpl.h"
#include "globals.h"
#include "config.h"

#include "rules/rules.h"

#include "input/inputs.h"

#include <sys/types.h>

#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>

using namespace UANLZ::JSONALERT;

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
        // Start input threads...
        Input::Inputs::startThreads();

        std::thread(RPCImpl::runRPClient).detach();

        Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,  (globalArguments->getDaemonName() + " initialized with PID: %d").c_str(), getpid());
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

        globalArguments->addCommandLineOption("Service Options", 'c', "config-dir" , "Configuration directory"  , "/etc/uauditanalyzer/" + globalArguments->getDaemonName(), Mantids::Memory::Abstract::TYPE_STRING );
    }

    bool _config(int , char *argv[], Arguments::GlobalArguments * globalArguments)
    {
        Mantids::Network::TLS::Socket_TLS::prepareTLS();

        std::string configDir = globalArguments->getCommandLineOptionValue("config-dir")->toString();

        // process config:
        unsigned int logMode = Logs::MODE_STANDARD;

        Logs::AppLog initLog(Logs::MODE_STANDARD);
        initLog.setPrintEmptyFields(true);
        initLog.setUsingAttributeName(false);
        initLog.setUserAlignSize(1);

        // start program.
        initLog.log(__func__, "","", Logs::LEVEL_INFO, 2048, "Starting... (Build date %s %s), PID: %u",__DATE__, __TIME__, getpid());
        initLog.log0(__func__,Logs::LEVEL_INFO, "Using config dir: %s", configDir.c_str());
        initLog.log0(__func__,Logs::LEVEL_INFO, "Loading configuration: %s", (configDir + "/config.ini").c_str());

        boost::property_tree::ptree config_main;

        if (access(configDir.c_str(),R_OK))
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Missing configuration dir: %s", configDir.c_str());
            return false;
        }

        chdir(configDir.c_str());

        if (!access((configDir + "/config.ini").c_str(),R_OK))
            boost::property_tree::ini_parser::read_ini( (configDir + "/config.ini").c_str(),config_main);
        else
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Missing configuration: %s", (configDir + "/config.ini").c_str());
            return false;
        }

        *(Globals::getConfig_main()) = config_main;

        if ( config_main.get<bool>("Logs.ToSyslog",true) ) logMode|=Logs::MODE_SYSLOG;
        Globals::setAppLog(new Logs::AppLog(logMode));
        Globals::getAppLog()->setPrintEmptyFields(true);
        Globals::getAppLog()->setUsingColors(config_main.get<bool>("Logs.ShowColors",true));
        Globals::getAppLog()->setUsingPrintDate(config_main.get<bool>("Logs.ShowDate",true));
        Globals::getAppLog()->setUserAlignSize(1);
        Globals::getAppLog()->setUsingAttributeName(false);
        Globals::getAppLog()->setDebug(Globals::getConfig_main()->get<bool>("Logs.Debug",false));

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Input::Inputs::setConfigFilePath(Globals::getConfig_main()->get<std::string>("Input.InputsPath","inputs.json"));
        Filters::Rules::setRulesFilePath(Globals::getConfig_main()->get<std::string>("Filters.RulesPath","rules.json"));
        Filters::Rules::setActionsFilePath(Globals::getConfig_main()->get<std::string>("Filters.ActionsPath","actions.json"));

        Filters::Rules::setEvaluationQueueMaxSize( Globals::getConfig_main()->get<size_t>("Filters.QueueSize",100000) );
        Filters::Rules::setMaxQueuePushWaitTimeInMilliseconds( Globals::getConfig_main()->get<uint64_t>("Filters.QueueMaxPushWaitTimeInMS",0) );
        Filters::Rules::setMaxQueuePopTimeInMilliseconds( Globals::getConfig_main()->get<uint64_t>("Filters.ThreadMaxPopTimeInMS",10000) );
        Filters::Rules::reloadActions(  );
        Filters::Rules::reloadRules(  );
        Filters::Rules::startEvaluationThreads( Globals::getConfig_main()->get<uint32_t>("Filters.Threads",16) );

        if (!Input::Inputs::loadConfig())
        {
            initLog.log0(__func__,Logs::LEVEL_CRITICAL, "Error loading inputs configuration.");
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

