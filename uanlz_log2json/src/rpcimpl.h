#ifndef RPCIMPL_H
#define RPCIMPL_H

#include <mdz_hlp_functions/json.h>

namespace UANLZ { namespace LOG2JSON {

class RPCImpl
{
public:
    RPCImpl();
    static void runRPClient();

private:
    static json statEventsPerHost(void *, const std::string &, const json &);
    static json statOutputsDistributionThreads(void *,const std::string &,  const json &);
    static json statOutputs(void *,const std::string &, const json &);
    static json statInputs(void *,const std::string &,  const json &);
//    static json helloWorld(void *,  const json &);
};

}}

#endif // RPCIMPL_H
