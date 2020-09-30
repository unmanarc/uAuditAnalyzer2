#ifndef WEBSERVERIMPL_H
#define WEBSERVERIMPL_H

#include <json/json.h>
#include <cx2_auth/iauth.h>
#include <cx2_auth/iauth_session.h>
#include <cx2_net_sockets/streamsocket.h>

namespace UANLZ { namespace WEB {

class WebServerImpl
{
public:
    WebServerImpl();
    static bool createWebServer();

private:
    static bool protoInitFail(void *webServer, CX2::Network::Streams::StreamSocket *sock, const char *remoteIP, bool isSecure);

    static Json::Value statMethods(void *, CX2::Authorization::IAuth *, CX2::Authorization::Session::IAuth_Session *, const Json::Value &);
    static Json::Value controlMethods(void *, CX2::Authorization::IAuth *, CX2::Authorization::Session::IAuth_Session *, const Json::Value &);
};

}}

#endif // WEBSERVERIMPL_H
