#include "tcpserver.h"

#include "../globals.h"
#include "../input/tcplineprocessor.h"

#include <mdz_net_sockets/socket_tcp.h>
#include <mdz_net_sockets/acceptor_multithreaded.h>

#include <boost/property_tree/ini_parser.hpp>
#include <inttypes.h>

using namespace std;
using namespace UANLZ::LOG2JSON::Input;
using namespace UANLZ::LOG2JSON;

using namespace Mantids::Memory::Streams;
using namespace Mantids::Network::Sockets;
using namespace Mantids::Network::Sockets;
using namespace Mantids::Application;

bool logServerThr(void * obj, Socket_StreamBase * baseClientSocket, const char * remotePair, bool secure)
{
    TCPServer * server = (TCPServer *)obj;
    TCPLineProcessor *  lineServer = new TCPLineProcessor(baseClientSocket,server);

    string sRemotePair = remotePair;
    lineServer->setRemoteIP(sRemotePair);

    string threadName = "IN_" + sRemotePair;
#ifndef WIN32
    pthread_setname_np(pthread_self(), threadName.c_str());
#endif

    LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Receiving %srsyslog+auditd incomming connection...", secure? "secure ":"");

    server->addClient(lineServer);

    // Process the line.
    Parser::ErrorMSG err;

    lineServer->parseObject(&err);
    //
    switch (err)
    {
    case Parser::PARSING_SUCCEED:
        LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Incomming connection finished.");
        break;
    case Parser::PARSING_ERR_INIT:
        LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PARSING_ERR_INIT");
        break;
    case Parser::PARSING_ERR_READ:
        LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PARSING_ERR_READ");
        break;
    case Parser::PARSING_ERR_PARSE:
        LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_WARN,"Connection finished with PARSING_ERR_PARSE");
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
        LOG_APP->log0(__func__,Logs::LEVEL_WARN,"Failed to load receptor configuration file %s, insufficient permissions?", file.c_str());
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
    Acceptors::MultiThreaded * vstreamer_syslog = new Acceptors::MultiThreaded();

    Socket_TCP * tcpServer = new Socket_TCP;
    if (!tcpServer->listenOn(listenPort,listenAddr.c_str(),true))
    {
        LOG_APP->log0(__func__,Logs::LEVEL_ERR,"Error creating TCP LOG listener @%s:%" PRIu16,listenAddr.c_str(),listenPort);
        delete vstreamer_syslog;
        delete tcpServer;
        return;
    }

    LOG_APP->log0(__func__,Logs::LEVEL_WARN,"LOG Listener (with decoder:%s) listener running on TCP @%s:%" PRIu16 "...", decoder.c_str(), listenAddr.c_str(),tcpServer->getPort());

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

