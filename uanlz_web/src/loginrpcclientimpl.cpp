#include "loginrpcclientimpl.h"
#include "globals.h"
#include <cx2_prg_logs/applog.h>

using namespace CX2;
using namespace UANLZ::WEB;
using namespace CX2::Application;

LoginRPCClientImpl::LoginRPCClientImpl()
{

}

LoginRPCClientImpl::~LoginRPCClientImpl()
{

}

void LoginRPCClientImpl::notifyTLSConnecting(CX2::Network::TLS::Socket_TLS *, const std::string &host, const uint16_t &port)
{
    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO, "Connecting to RPC Login Server [%s:%d]...", host.c_str(), port);
}

void LoginRPCClientImpl::notifyTLSDisconnected(Network::TLS::Socket_TLS *, const std::string &host, const uint16_t &port, int code)
{
    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO, "Disconnected from RPC Login Server [%s:%d] with code %d...", host.c_str(), port, code);
}

void LoginRPCClientImpl::notifyAPIProcessingOK(CX2::Network::TLS::Socket_TLS *tlsSock)
{
    char remotePair[INET6_ADDRSTRLEN+1];
    memset(remotePair,0,INET6_ADDRSTRLEN+1);
    tlsSock->getRemotePair(remotePair);

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO, "Connected to RPC Login Server [%s:%d] via TLS, authenticated.", remotePair, tlsSock->getRemotePort());
}

void LoginRPCClientImpl::notifyTLSConnectedOK(CX2::Network::TLS::Socket_TLS *tlsSock)
{
    char remotePair[INET6_ADDRSTRLEN+1];
    memset(remotePair,0,INET6_ADDRSTRLEN+1);
    tlsSock->getRemotePair(remotePair);

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO, "Connected to RPC Login Server [%s:%d] via TLS, starting authentication...", remotePair, tlsSock->getRemotePort());
}

void LoginRPCClientImpl::notifyBadApiKey(CX2::Network::TLS::Socket_TLS *tlsSock)
{
    char remotePair[INET6_ADDRSTRLEN+1];
    memset(remotePair,0,INET6_ADDRSTRLEN+1);
    tlsSock->getRemotePair(remotePair);

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Disconnected from RPC Login Server [%s:%d] via TLS, bad api key.", remotePair, tlsSock->getRemotePort());
}

void LoginRPCClientImpl::notifyTLSErrorConnecting(CX2::Network::TLS::Socket_TLS *tlsSock, const std::string &host, const uint16_t &port)
{
    Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "Error connecting to RPC Login Server [%s:%d] via TLS: %s", host.c_str(),port, tlsSock->getLastError().c_str());

    for (const auto & i :tlsSock->getTLSErrorsAndClear())
    {
        if (!strstr(i.c_str(),"certificate unknown"))
            Globals::getAppLog()->log0(__func__,Logs::LEVEL_ERR, "RPC Login TLS Protocol Initialization Error [%s:%d]: %s",  host.c_str(),port,i.c_str());
    }
}
