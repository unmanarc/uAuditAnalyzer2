#ifndef LOGINRPCCLIENTIMPL_H
#define LOGINRPCCLIENTIMPL_H

#include <cx2_auth_remote/loginrpcclient.h>

class LoginRPCClientImpl : public CX2::Authentication::LoginRPCClient
{
public:
    LoginRPCClientImpl();
    virtual ~LoginRPCClientImpl();

    void notifyTLSConnecting(CX2::Network::TLS::Socket_TLS *, const std::string & host, const uint16_t &port);
    void notifyTLSDisconnected(CX2::Network::TLS::Socket_TLS * , const std::string & , const uint16_t &, int code);
    void notifyAPIProcessingOK(CX2::Network::TLS::Socket_TLS * tlsSock);
    void notifyTLSConnectedOK(CX2::Network::TLS::Socket_TLS * tlsSock);
    void notifyBadApiKey(CX2::Network::TLS::Socket_TLS * tlsSock);
    void notifyTLSErrorConnecting(CX2::Network::TLS::Socket_TLS * tlsSock, const std::string &host, const uint16_t &port);

};


#endif // LOGINRPCCLIENTIMPL_H
