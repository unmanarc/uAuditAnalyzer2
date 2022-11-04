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
    static json controlWriteRulesFileConfig(void *, const std::string &, const json &jConfig, void*, const std::string &);
    static json statReadRulesFileConfig(void *,const std::string &, const json &, void*, const std::string &);
    static json statReadRunningRules(void *,const std::string &, const json &, void*, const std::string &);
    static json statReadRunningRule(void *,const std::string &, const json &, void*, const std::string &);
    static json statAreRulesModified(void *, const std::string &, const json &, void*, const std::string &);

    static json controlRemoveRule(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlRemoveAction(void *,const std::string &, const json &jConfig, void*, const std::string &);

    static json controlSaveRunningRules(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlSaveRunningActions(void *,const std::string &, const json &jConfig, void*, const std::string &);

    static json controlAddAction(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlEditAction(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlEditRule(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlAddRule(void *,const std::string &, const json &jConfig, void*, const std::string &);

    static json controlRuleUp(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json controlRuleDown(void *,const std::string &, const json &jConfig, void*, const std::string &);

    static json controlWriteActionsFileConfig(void *,const std::string &, const json &jConfig, void*, const std::string &);
    static json statAreActionsModified(void *, const std::string &, const json &, void*, const std::string &);
    static json statReadActionsFileConfig(void *,const std::string &, const json &, void*, const std::string &);
    static json statReadRunningActions(void *,const std::string &, const json &, void*, const std::string &);
    static json statReadRunningAction(void *,const std::string &, const json &, void*, const std::string &);


    static json RemInputFromFileConfig(void *, const std::string &, const json &jConfig, void*, const std::string &);
    static json AddInputToFileConfig(void *, const std::string &, const json &jConfig, void*, const std::string &);
    static json statReadInputsFileConfig(void *,const std::string &, const json &, void*, const std::string &);
    static json statGetRunningInputs(void *,const std::string &, const json &, void*, const std::string &);
    static json controlReloadApp(void *,const std::string &, const json &, void*, const std::string &);

    static json statSystemStats(void *, const std::string &, const json &, void*, const std::string &);
    static json controlRulesReload(void *, const std::string &, const json &, void*, const std::string &);
    static json controlActionsReload(void *, const std::string &, const json &, void*, const std::string &);
};

}}

#endif // RPCIMPL_H
