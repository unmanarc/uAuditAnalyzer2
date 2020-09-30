#ifndef STATUS_H
#define STATUS_H

#include <json/json.h>
#include <cx2_auth/iauth.h>
#include <cx2_auth/iauth_session.h>

namespace UANLZ { namespace WEB {  namespace WebService {

Json::Value statInputs(void *,CX2::Authorization::IAuth *, CX2::Authorization::Session::IAuth_Session *, const Json::Value &);
Json::Value statRules(void *,CX2::Authorization::IAuth *, CX2::Authorization::Session::IAuth_Session *, const Json::Value &);
Json::Value controlRulesReload(void *,CX2::Authorization::IAuth *, CX2::Authorization::Session::IAuth_Session *, const Json::Value &);
}}}

#endif // STATUS_H
