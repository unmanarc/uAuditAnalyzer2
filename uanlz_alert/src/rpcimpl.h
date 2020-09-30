#ifndef RPCIMPL_H
#define RPCIMPL_H

#include <json/json.h>

namespace UANLZ { namespace JSONALERT {

class RPCImpl
{
public:
    RPCImpl();
    static void runRPClient();

private:
    static Json::Value statInputs(void *, const Json::Value &);
    static Json::Value statRules(void *, const Json::Value &);
    static Json::Value controlRulesReload(void *, const Json::Value &);


/*    static Json::Value statEventsPerHost(void *, const Json::Value &);
    static Json::Value statOutputsDistributionThreads(void *,  const Json::Value &);
    static Json::Value statOutputs(void *, const Json::Value &);
    static Json::Value statInputs(void *,  const Json::Value &);*/
//    static Json::Value helloWorld(void *,  const Json::Value &);
};

}}

#endif // RPCIMPL_H
