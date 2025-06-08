#include "tcpserver.h"

#include "../globals.h"
#include "../input/tcplineprocessor.h"

#include <mdz_net_sockets/socket_tcp.h>

#include <boost/property_tree/ini_parser.hpp>
#include <inttypes.h>

using namespace std;
using namespace UANLZ::JSONALERT;
using namespace UANLZ::JSONALERT::Input;

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
    pthread_setname_np(pthread_self(), threadName.c_str());

    LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Receiving %sJSONTCP Incoming connection...", secure? "secure " : "");

    server->addClient(lineServer);

    // Process the line.
    Parser::ErrorMSG err;
    lineServer->parseObject(&err);
    //
    switch (err)
    {
    case Parser::PARSING_SUCCEED:
        LOG_APP->log1(__func__,sRemotePair,Logs::LEVEL_INFO,"Incoming connection finished.");
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
}

bool TCPServer::loadConfig(const json & jConfig)
{

    listenAddr = JSON_ASSTRING(jConfig,"ListenAddr","");
    listenPort = JSON_ASUINT(jConfig,"ListenPort",0);
    description = JSON_ASSTRING(jConfig,"Description","");

    LOG_APP->log0(__func__,Logs::LEVEL_INFO,"Configuring JSON input '%s' @%s:%" PRIu16,description.c_str(),listenAddr.c_str(),listenPort );


    return true;
}

void TCPServer::startThreaded()
{
    Socket_TCP * tcpListener = new Socket_TCP();

    if (!tcpListener->listenOn(listenPort,listenAddr.c_str(),true))
    {
        LOG_APP->log0(__func__,Logs::LEVEL_ERR,"Error creating JSON TCP listener @%s:%" PRIu16,listenAddr.c_str(),listenPort);
        delete tcpListener;
        return;
    }

    LOG_APP->log0(__func__,Logs::LEVEL_WARN,"JSON TCP Listener listener running @%s:%" PRIu16 "...",  listenAddr.c_str(),tcpListener->getPort());

    // STREAM MANAGER:
    acceptor.setAcceptorSocket(tcpListener);
    acceptor.setCallbackOnConnect(&logServerThr, this);
    acceptor.startThreaded();
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
