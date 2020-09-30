#include "rpcserverimpl.h"
#include "globals.h"

#include <cx2_net_sockets/socket_tls.h>
#include <cx2_net_sockets/socket_acceptor_multithreaded.h>
#include <cx2_xrpc_fast/fastrpc.h>
#include <cx2_prg_logs/applog.h>
#include <string>

using namespace UANLZ::WEB;

using namespace CX2::Application;
using namespace CX2::RPC;
using namespace CX2;

RPCServerImpl::RPCServerImpl()
{
}

bool RPCServerImpl::callbackOnRPCConnect(void *, CX2::Network::Streams::StreamSocket *sock, const char * remoteAddr, bool secure)
{
    std::string rpcApiKey = sock->readString(nullptr,16);

    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO, "Incomming %sRPC connection from %s", secure?"secure ":"", remoteAddr);

    if (rpcApiKey == Globals::getConfig_main()->get<std::string>("RPCServer.AlertsApiKey","REPLACEME_XABCXAPIX_ALERTS"))
    {
        if (Globals::getFastRPC()->processConnection(sock,"ALERTS")==-2)
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "ALERTS RPC Already Connected, giving up from %s.", remoteAddr);
        }
    }
    else if (rpcApiKey == Globals::getConfig_main()->get<std::string>("RPCServer.Log2JSONApiKey","REPLACEME_XABCXAPIX_LOG2JSON"))
    {
        if (Globals::getFastRPC()->processConnection(sock,"LOG2JSON")==-2)
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR, "LOG2JSON RPC Already Connected, giving up from %s.", remoteAddr);
        }
    }

    return true;
}

bool RPCServerImpl::createRPCListener()
{
    CX2::Network::TLS::Socket_TLS * sockRPCListen = new CX2::Network::TLS::Socket_TLS;

    uint16_t listenPort = Globals::getConfig_main()->get<uint16_t>("RPCServer.ListenPort",33001);
    std::string listenAddr = Globals::getConfig_main()->get<std::string>("RPCServer.ListenAddr","0.0.0.0");

    if (!sockRPCListen->setTLSPublicKeyPath( Globals::getConfig_main()->get<std::string>("RPCServer.CertFile","snakeoil.crt").c_str() ))
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting RPC Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS RPC Server Public Key");
        return false;
    }
    if (!sockRPCListen->setTLSPrivateKeyPath( Globals::getConfig_main()->get<std::string>("RPCServer.KeyFile","snakeoil.key").c_str() ))
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting RPC Server @%s:%d: %s", listenAddr.c_str(), listenPort, "Bad TLS RPC Server Private Key");
        return false;
    }

    Globals::setFastRPC(new CX2::RPC::Fast::FastRPC);

    Network::Sockets::Acceptors::Socket_Acceptor_MultiThreaded * multiThreadedAcceptor = new Network::Sockets::Acceptors::Socket_Acceptor_MultiThreaded;
    multiThreadedAcceptor->setMaxConcurrentClients( Globals::getConfig_main()->get<uint16_t>("RPCServer.MaxClients",512) );
    multiThreadedAcceptor->setCallbackOnConnect(callbackOnRPCConnect,nullptr);

    if (sockRPCListen->listenOn(listenPort ,listenAddr.c_str(), !Globals::getConfig_main()->get<bool>("RPCServer.ipv6",false) ))
    {
        multiThreadedAcceptor->setAcceptorSocket(sockRPCListen);
        multiThreadedAcceptor->startThreaded();
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,  "Accepting RPC clients @%s:%d via TLS", listenAddr.c_str(), listenPort);
        return true;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL, "Error starting RPC Server @%s:%d: %s", listenAddr.c_str(), listenPort, sockRPCListen->getLastError());
        return false;
    }
}



