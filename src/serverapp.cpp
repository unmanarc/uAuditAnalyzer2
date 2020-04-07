#include "serverapp.h"

#include "status.h"

#include "output/output_jsontcp.h"
#include "output/output_postgresql.h"
#include "output/processorthreads_output.h"
#include "input/tcplineprocessor.h"
#include "vars/namedefs.h"
#include "rules/rules.h"
#include "globals.h"

#include <Poco/Format.h>
#include <Poco/Util/HelpFormatter.h>

#include <cx_net_sockets/socket_tcp.h>
#include <cx_net_threadedacceptor/threadedstreamacceptor.h>

using namespace std;

using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Exception;

using namespace std;

static string currentDir = "";

#define SERVERAPP ((ServerAPP *)Globals::getServerApp())

bool rsyslogAuditdServerThread(void *, Socket_Base_Stream * baseClientSocket, const char * remotePair)
{
    TCPLineProcessor lineServer(baseClientSocket,true);   

    std::string sRemotePair = remotePair;
    lineServer.setRemoteIP(sRemotePair);

    std::string threadName = "IN_" + sRemotePair;
    pthread_setname_np(pthread_self(), threadName.c_str());

    SERVERAPP->getLogger()->information("Starting rsyslog+auditd incomming connection from @%s...", sRemotePair);

    // Process the line.
    ParseErrorMSG err;
    // TODO: all the try/catch for db should be done here.
    lineServer.parseObject(&err);
    //
    switch (err)
    {
    case PROT_PARSER_SUCCEED:
        SERVERAPP->getLogger()->information("Connection from %s finished.",sRemotePair);
        break;
    case PROT_PARSER_ERR_INIT:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_INIT",sRemotePair);
        break;
    case PROT_PARSER_ERR_READ:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_READ",sRemotePair);
        break;
    case PROT_PARSER_ERR_PARSE:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_PARSE",sRemotePair);
        break;
    }

    return true;
}

bool pureAuditdServerThread(void *, Socket_Base_Stream * baseClientSocket, const char * remotePair)
{
    TCPLineProcessor lineServer(baseClientSocket,false);
    std::string sRemotePair = remotePair;

    SERVERAPP->getLogger()->information("Starting pure auditd incomming connection from @%s...", sRemotePair);

    // Process the line.
    ParseErrorMSG err;
    // TODO: all the try/catch for db should be done here.
    lineServer.parseObject(&err);
    //
    switch (err)
    {
    case PROT_PARSER_SUCCEED:
        SERVERAPP->getLogger()->information("Connection from %s finished.",sRemotePair);
        break;
    case PROT_PARSER_ERR_INIT:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_INIT",sRemotePair);
        break;
    case PROT_PARSER_ERR_READ:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_READ",sRemotePair);
        break;
    case PROT_PARSER_ERR_PARSE:
        SERVERAPP->getLogger()->warning("Connection from %s finished with PROT_PARSER_ERR_PARSE",sRemotePair);
        break;
    }

    return true;
}


ServerAPP::ServerAPP(): _helpRequested(false)
{
    string remoteHost;
}

Poco::Logger *ServerAPP::getLogger()
{
    return &(Application::instance().logger());
}

ServerAPP::~ServerAPP()
{
}

void ServerAPP::initialize(Poco::Util::Application &self)
{
    loadConfiguration(); // load default configuration files, if present
    ServerApplication::initialize(self);
}

void ServerAPP::uninitialize()
{
    ServerApplication::uninitialize();
}

void ServerAPP::reinitialize(Application& self)
{
    Application::reinitialize(self);
    // add your own reinitialization code here
}

void ServerAPP::defineOptions(OptionSet &options)
{
    ServerApplication::defineOptions(options);

    options.addOption(
                Option("help", "h", "display help information on command line arguments")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<ServerAPP>(this, &ServerAPP::handleHelp)));

    options.addOption(
                Option("config-file", "c", "load configuration data from a file")
                .required(true)
                .repeatable(true)
                .argument("file")
                .callback(OptionCallback<ServerAPP>(this, &ServerAPP::handleConfig)));

    options.addOption(
                Option("rules-dir", "r", "load rules configuration from directory")
                .required(false)
                .repeatable(true)
                .argument("file")
                .callback(OptionCallback<ServerAPP>(this, &ServerAPP::handleRules)));
}

void ServerAPP::handleHelp(const string &, const string &)
{
    _helpRequested = true;
    displayHelp();
    stopOptionsProcessing();
}

void ServerAPP::handleConfig(const string& , const string& value)
{
//    loadConfiguration(value);
    Application& app = Application::instance();

    app.logger().information("Loading configuration file: %s", value);

    boost::property_tree::ptree config_main;

    if (!access(value.c_str(),R_OK))
        boost::property_tree::ini_parser::read_ini( value.c_str(),config_main);
    else
    {
        app.logger().critical("Missing configuration file: %s", value);
        _exit(-1);
    }

    *(Globals::getConfig_main()) = config_main;
}

void ServerAPP::handleRules(const string& , const string& value)
{
    Application& app = Application::instance();
    app.logger().information("Loading rules directory: %s", value);
    Rules::reload( value );
}

void ServerAPP::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("An auditd parser/decoder for logs comprehensive analysis");
    helpFormatter.format(cout);
}

int ServerAPP::main(const vector<string> &)
{
    NameDefs::init();

    Application& app = Application::instance();
    if (!_helpRequested)
    {
        // Server Mode.

        app.logger().setLevel( Globals::getConfig_main()->get<std::string>("Logs.Level","debug") );
        app.logger().information("Starting auditd analyzer...");

        // Creating outputs...
        if (Globals::getConfig_main()->get<bool>("Stats.Enabled",true))
        {
            Status::startThreaded();
        }

        if (Globals::getConfig_main()->get<bool>("OUTPUT/PostgreSQL.Enabled",false))
            Globals::addOutputBaseAndStartThreads(new Output_ProgreSQL());
        if (Globals::getConfig_main()->get<bool>("OUTPUT/JSONTCP.Enabled",true))
            Globals::addOutputBaseAndStartThreads(new Output_JSONTCP());

        EventsManager::startGC();

        ProcessorThreads_Output::startProcessorThreads( Globals::getConfig_main()->get<size_t>("Processor.Threads",8) );
        ProcessorThreads_Output::setQueueSize(Globals::getConfig_main()->get<size_t>("Processor.QueueSize",1000));
        ThreadedStreamAcceptor vstreamer_syslog;
        ThreadedStreamAcceptor vstreamer_auditd;       

        if (Globals::getConfig_main()->get<bool>("INPUT/SYSLOG.Enabled",true))
        {
            // NOW LISTEN...
            std::string listenAddr = Globals::getConfig_main()->get<std::string>("INPUT/SYSLOG.ListenAddr","127.0.0.1");
            uint16_t listenPort = Globals::getConfig_main()->get<uint16_t>("INPUT/SYSLOG.Port",10514);
            Socket_TCP * tcpServer = new Socket_TCP;
            if (!tcpServer->listenOn(listenPort,listenAddr.c_str(),true))
            {
                app.logger().error("Error creating rsyslog+auditd listener @%s:%d",listenAddr,(int)listenPort);
                return Application::EXIT_PROTOCOL;
            }
            app.logger().information("Rsyslog+auditd analyzer listener running @%s:%d...", listenAddr,(int)tcpServer->getPort());

            Globals::setServerApp(this);

            // STREAM MANAGER:
            vstreamer_syslog.setAcceptorSocket(tcpServer);
            vstreamer_syslog.setCallbackOnConnect(&rsyslogAuditdServerThread, nullptr);
            vstreamer_syslog.startThreaded();
        }

        if (Globals::getConfig_main()->get<bool>("INPUT/PUREAUDIT.Enabled",true))
        {
            // NOW LISTEN...
            std::string listenAddr = Globals::getConfig_main()->get<std::string>("INPUT/PUREAUDIT.ListenAddr","127.0.0.1");
            uint16_t listenPort = Globals::getConfig_main()->get<uint16_t>("INPUT/PUREAUDIT.Port",10060);
            Socket_TCP * tcpServer = new Socket_TCP;
            if (!tcpServer->listenOn(listenPort,listenAddr.c_str(),true))
            {
                app.logger().error("Error creating pure auditd server @%s:%d",listenAddr,(int)listenPort);
                return Application::EXIT_PROTOCOL;
            }
            app.logger().information("Pure auditd analyzer listener running @%s:%d...", listenAddr,(int)tcpServer->getPort());

            Globals::setServerApp(this);

            // STREAM MANAGER:
            vstreamer_auditd.setAcceptorSocket(tcpServer);
            vstreamer_auditd.setCallbackOnConnect(&pureAuditdServerThread, nullptr);
            vstreamer_auditd.startThreaded();
        }

        // wait for CTRL-C or kill
        waitForTerminationRequest();
        // Stop the ...
    }
    return Application::EXIT_OK;
}
