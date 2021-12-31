#ifndef RPCIMPL_H
#define RPCIMPL_H

#include <mdz_hlp_functions/json.h>

namespace UANLZ { namespace JSONALERT {

class RPCImpl
{
public:
    RPCImpl();
    static void runRPClient();

private:
    static json controlWriteRulesFileConfig(void *, const std::string &, const json &jConfig);
    static json statReadRulesFileConfig(void *,const std::string &, const json &);
    static json statReadRunningRules(void *,const std::string &, const json &);
    static json statReadRunningRule(void *,const std::string &, const json &);
    static json statAreRulesModified(void *, const std::string &, const json &);

    static json controlRemoveRule(void *,const std::string &, const json &jConfig);
    static json controlRemoveAction(void *,const std::string &, const json &jConfig);

    static json controlSaveRunningRules(void *,const std::string &, const json &jConfig);
    static json controlSaveRunningActions(void *,const std::string &, const json &jConfig);

    static json controlAddAction(void *,const std::string &, const json &jConfig);
    static json controlEditAction(void *,const std::string &, const json &jConfig);
    static json controlEditRule(void *,const std::string &, const json &jConfig);
    static json controlAddRule(void *,const std::string &, const json &jConfig);

    static json controlRuleUp(void *,const std::string &, const json &jConfig);
    static json controlRuleDown(void *,const std::string &, const json &jConfig);

    static json controlWriteActionsFileConfig(void *,const std::string &, const json &jConfig);
    static json statAreActionsModified(void *, const std::string &, const json &);
    static json statReadActionsFileConfig(void *,const std::string &, const json &);
    static json statReadRunningActions(void *,const std::string &, const json &);
    static json statReadRunningAction(void *,const std::string &, const json &);


    static json RemInputFromFileConfig(void *, const std::string &, const json &jConfig);
    static json AddInputToFileConfig(void *, const std::string &, const json &jConfig);
    static json statReadInputsFileConfig(void *,const std::string &, const json &);
    static json statGetRunningInputs(void *,const std::string &, const json &);
    static json controlReloadApp(void *,const std::string &, const json &);

    static json statSystemStats(void *, const std::string &, const json &);
    static json controlRulesReload(void *, const std::string &, const json &);
    static json controlActionsReload(void *, const std::string &, const json &);
};

}}

#endif // RPCIMPL_H
