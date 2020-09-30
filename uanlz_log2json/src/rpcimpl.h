#ifndef RPCIMPL_H
#define RPCIMPL_H

#include <json/json.h>

namespace UANLZ { namespace LOG2JSON {

class RPCImpl
{
public:
    RPCImpl();
    static void runRPClient();

private:
    static Json::Value statEventsPerHost(void *, const Json::Value &);
    static Json::Value statOutputsDistributionThreads(void *,  const Json::Value &);
    static Json::Value statOutputs(void *, const Json::Value &);
    static Json::Value statInputs(void *,  const Json::Value &);
//    static Json::Value helloWorld(void *,  const Json::Value &);
};

}}

#endif // RPCIMPL_H
