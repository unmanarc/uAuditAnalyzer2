#ifndef LOGINRPCCLIENTIMPL_H
#define LOGINRPCCLIENTIMPL_H

#include <mdz_auth_remote/loginrpcclient.h>

class LoginRPCClientImpl : public Mantids::Authentication::LoginRPCClient
{
public:
    LoginRPCClientImpl();
    virtual ~LoginRPCClientImpl();

    void notifyTLSConnecting(Mantids::Network::Sockets::Socket_TLS *, const std::string & host, const uint16_t &port);
    void notifyTLSDisconnected(Mantids::Network::Sockets::Socket_TLS * , const std::string & , const uint16_t &, int code);
    void notifyAPIProcessingOK(Mantids::Network::Sockets::Socket_TLS * tlsSock);
    void notifyTLSConnectedOK(Mantids::Network::Sockets::Socket_TLS * tlsSock);
    void notifyBadApiKey(Mantids::Network::Sockets::Socket_TLS * tlsSock);
    void notifyTLSErrorConnecting(Mantids::Network::Sockets::Socket_TLS * tlsSock, const std::string &host, const uint16_t &port);

};


#endif // LOGINRPCCLIENTIMPL_H
