#include "tcpserver.h"

#include "../globals.h"
#include "../input/tcplineprocessor.h"

#include <mdz_net_sockets/socket_tcp.h>
#include <mdz_net_sockets/socket_acceptor_multithreaded.h>

#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace UANLZ::JSONALERT;
using namespace UANLZ::JSONALERT::Input;

using namespace Mantids::Memory::Streams;
using namespace Mantids::Network::Streams;
using namespace Mantids::Network::Sockets;
using namespace Mantids::Application;

bool logServerThr(void * obj, StreamSocket * baseClientSocket, const char * remotePair, bool secure)
{
    TCPServer * server = (TCPServer *)obj;
    TCPLineProcessor *  lineServer = new TCPLineProcessor(baseClientSocket,server);

    string sRemotePair = remotePair;
    lineServer->setRemoteIP(sRemotePair);

    string threadName = "IN_" + sRemotePair;
    pthread_setname_np(pthread_self(), threadName.c_str());

    Globals::getAppLog()->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Receiving %sJSONTCP Incomming connection...", secure? "secure " : "");

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
}

bool TCPServer::loadConfig(const json & jConfig)
{

    listenAddr = JSON_ASSTRING(jConfig,"ListenAddr","");
    listenPort = JSON_ASUINT(jConfig,"ListenPort",0);
    description = JSON_ASSTRING(jConfig,"Description","");

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,"Configuring JSON input '%s' @%s:%d",description.c_str(),listenAddr.c_str(),listenPort );


    return true;
}

void TCPServer::startThreaded()
{
    Acceptors::Socket_Acceptor_MultiThreaded * vstreamer_syslog = new Acceptors::Socket_Acceptor_MultiThreaded;

    Socket_TCP * tcpServer = new Socket_TCP;
    if (!tcpServer->listenOn(listenPort,listenAddr.c_str(),true))
    {
        Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR,"Error creating JSON TCP listener @%s:%lu",listenAddr.c_str(),listenPort);
        return;
    }

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_WARN,"JSON TCP Listener listener running @%s:%lu...",  listenAddr.c_str(),tcpServer->getPort());

    // STREAM MANAGER:
    vstreamer_syslog->setAcceptorSocket(tcpServer);
    vstreamer_syslog->setCallbackOnConnect(&logServerThr, this);
    vstreamer_syslog->startThreaded();
}

json TCPServer::getStats()
{
    json v;

    v["description"] = description;

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
