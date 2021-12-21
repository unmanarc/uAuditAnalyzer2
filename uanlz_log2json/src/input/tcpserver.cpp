#include "tcpserver.h"

#include "../globals.h"
#include "../input/tcplineprocessor.h"

#include <cx2_net_sockets/socket_tcp.h>
#include <cx2_net_sockets/socket_acceptor_multithreaded.h>

#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace UANLZ::LOG2JSON::Input;
using namespace UANLZ::LOG2JSON;

using namespace CX2::Memory::Streams;
using namespace CX2::Network::Streams;
using namespace CX2::Network::Sockets;
using namespace CX2::Application;

bool logServerThr(void * obj, StreamSocket * baseClientSocket, const char * remotePair, bool secure)
{
    TCPServer * server = (TCPServer *)obj;
    TCPLineProcessor *  lineServer = new TCPLineProcessor(baseClientSocket,server);

    string sRemotePair = remotePair;
    lineServer->setRemoteIP(sRemotePair);

    string threadName = "IN_" + sRemotePair;
    pthread_setname_np(pthread_self(), threadName.c_str());

    Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Receiving %srsyslog+auditd incomming connection...", secure? "secure ":"");

    server->addClient(lineServer);

    // Process the line.
    Parsing::ParseErrorMSG err;

    lineServer->parseObject(&err);
    //
    switch (err)
    {
    case Parsing::PROT_PARSER_SUCCEED:
        Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Incomming connection finished.");
        break;
    case Parsing::PROT_PARSER_ERR_INIT:
        Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PROT_PARSER_ERR_INIT");
        break;
    case Parsing::PROT_PARSER_ERR_READ:
        Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PROT_PARSER_ERR_READ");
        break;
    case Parsing::PROT_PARSER_ERR_PARSE:
        Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PROT_PARSER_ERR_PARSE");
        break;
    }

    server->remClient(lineServer);

    delete lineServer;

    return true;
}

TCPServer::TCPServer()
{
    invalidLinesCount = 0;
    processedLinesCount = 0;
}

bool TCPServer::loadConfig(const string &file)
{
    if (!access(file.c_str(),R_OK))
        boost::property_tree::ini_parser::read_ini( file.c_str(),config);
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_WARN,"Failed to load receptor configuration file %s, insufficient permissions?", file.c_str());
        return false;
    }

    listenAddr = config.get<string>("ListenAddr","127.0.0.1");
    listenPort = config.get<uint16_t>("ListenPort",10514);
    decoder = config.get<string>("Decoder", "SYSLOG+AUDITD");
    description = config.get<string>("Description", decoder + " log receiver @" + listenAddr + ":" + to_string(listenPort));

    return true;
}

void TCPServer::startThreaded()
{
    Acceptors::Socket_Acceptor_MultiThreaded * vstreamer_syslog = new Acceptors::Socket_Acceptor_MultiThreaded();

    Socket_TCP * tcpServer = new Socket_TCP;
    if (!tcpServer->listenOn(listenPort,listenAddr.c_str(),true))
    {
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR,"Error creating TCP LOG listener @%s:%lu",listenAddr.c_str(),listenPort);
        return;
    }

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_WARN,"LOG Listener (with decoder:%s) listener running on TCP @%s:%lu...", decoder.c_str(), listenAddr.c_str(),tcpServer->getPort());

    // STREAM MANAGER:
    vstreamer_syslog->setAcceptorSocket(tcpServer);
    vstreamer_syslog->setCallbackOnConnect(&logServerThr, this);
    vstreamer_syslog->startThreaded();
}

json TCPServer::getStats()
{
    json v;

    v["description"] = description;
    v["processedLinesCount"] = (Json::UInt64)processedLinesCount;
    v["invalidLinesCount"] = (Json::UInt64)invalidLinesCount;

    int x=0;
    mt.lock();
    for (auto i : clientProcessors)
    {
        v["clients"][x] = i->getStats();
        x++;
    }
    mt.unlock();

    return v;
}

void TCPServer::addClient(TCPLineProcessor *clientProcessor)
{
    mt.lock();
    clientProcessors.insert(clientProcessor);
    mt.unlock();
}

void TCPServer::remClient(TCPLineProcessor *clientProcessor)
{
    mt.lock();
    clientProcessors.erase(clientProcessor);
    mt.unlock();
}

string TCPServer::getListenAddr() const
{
    return listenAddr;
}

uint16_t TCPServer::getListenPort() const
{
    return listenPort;
}

string TCPServer::getDecoder() const
{
    return decoder;
}

void TCPServer::incProcessedLines()
{
    processedLinesCount++;
}

void TCPServer::incInvalidLines()
{
    invalidLinesCount++;
}

