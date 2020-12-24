#ifndef STATUS_H
#define STATUS_H

#include <json/json.h>
#include <cx2_auth/manager.h>
#include <cx2_auth/session.h>

namespace UANLZ { namespace WEB {  namespace WebService {

Json::Value statInputs(void *,CX2::Authentication::Manager *, CX2::Authentication::Session *, const Json::Value &);
Json::Value statRules(void *,CX2::Authentication::Manager *, CX2::Authentication::Session *, const Json::Value &);
Json::Value controlRulesReload(void *,CX2::Authentication::Manager *, CX2::Authentication::Session *, const Json::Value &);
}}}

#endif // STATUS_H
