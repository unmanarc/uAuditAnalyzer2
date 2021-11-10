#ifndef WEBSERVERIMPL_H
#define WEBSERVERIMPL_H

#include <cx2_hlp_functions/json.h>
#include <cx2_auth/manager.h>
#include <cx2_auth/session.h>
#include <cx2_net_sockets/streamsocket.h>

namespace UANLZ { namespace WEB {

class WebServerImpl
{
public:
    WebServerImpl();
    static bool createWebServer();

private:
    static bool protoInitFail(void *webServer, CX2::Network::Streams::StreamSocket *sock, const char *remoteIP, bool isSecure);

    static json statMethods(void *, CX2::Authentication::Manager *, CX2::Authentication::Session *, const json &);
    static json controlMethods(void *, CX2::Authentication::Manager *, CX2::Authentication::Session *, const json &);


    static json rmtCaller(const std::string & caller, CX2::Authentication::Manager *auth, CX2::Authentication::Session *sess, const json &jInput);

};

}}

#endif // WEBSERVERIMPL_H
