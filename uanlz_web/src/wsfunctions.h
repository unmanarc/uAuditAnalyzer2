#ifndef STATUS_H
#define STATUS_H

#include <json/json.h>
#include <mdz_auth/manager.h>
#include <mdz_auth/session.h>

namespace UANLZ { namespace WEB {  namespace WebService {

Json::Value statInputs(void *,Mantids::Authentication::Manager *, Mantids::Authentication::Session *, const Json::Value &);
Json::Value statRules(void *,Mantids::Authentication::Manager *, Mantids::Authentication::Session *, const Json::Value &);
Json::Value controlRulesReload(void *,Mantids::Authentication::Manager *, Mantids::Authentication::Session *, const Json::Value &);
}}}

#endif // STATUS_H
