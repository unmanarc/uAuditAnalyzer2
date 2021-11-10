#ifndef RULEACTION_H
#define RULEACTION_H

#include <vector>
#include <string>
#include <mutex>

#include <cx2_scripts_jsonexpreval/jsoneval.h>
#include <cx2_thr_mutex/mutex_shared.h>
#include <cx2_thr_safecontainers/queue.h>

#include <boost/property_tree/ptree.hpp>
#include <cx2_hlp_functions/json.h>

namespace UANLZ { namespace JSONALERT { namespace Filters {

enum eRuleAction {
    RULE_ACTION_EXEC,
    RULE_ACTION_ABORT,
    RULE_ACTION_EXECANDABORT,
    RULE_ACTION_UNDEFINED
};

struct sRule
{
    sRule()
    {
        ruleAction = RULE_ACTION_UNDEFINED;
        expr = nullptr;
    }
    ~sRule()
    {
        if (expr) delete expr;
    }

    eRuleAction ruleAction;

    std::string name;
    std::string actionId;

    std::string filter;
    CX2::Scripts::Expressions::JSONEval *expr;

    json jOriginalVal;
};

struct sAction
{
    sAction()
    {
        file = nullptr;
    }
    ~sAction()
    {
        if (file) free(file);
    }

    void setFileName(const std::string & sFileName)
    {
        file = strdup(sFileName.c_str());
    }

    void setArguments(const std::vector<std::string> & vArguments)
    {
        this->vArguments = vArguments;
    }

    char * file;
    std::vector<std::string> vArguments;
    json jOriginalVal;

};

class Rules
{
public:
    Rules();
    ~Rules();

    ////////////////////////////////////////////////////////////////////////////////
    // Stats:
    static json getStats();

    ////////////////////////////////////////////////////////////////////////////////
    // Threads:
    static bool startEvaluationThreads( const uint32_t & threadsCount );
    static void setEvaluationQueueMaxSize( const size_t & max );
    static void setMaxQueuePushWaitTimeInMilliseconds(const uint64_t &value);
    static void setMaxQueuePopTimeInMilliseconds(const uint64_t &value);
    static bool pushElementOnEvaluationQueue( const std::string & str );

    ////////////////////////////////////////////////////////////////////////////////
    // Rule evaluation & Action Triggering:
    static bool evaluate( const json & values );

    ////////////////////////////////////////////////////////////////////////////////
    // Config Rules/Actions:
    static bool editRule(uint32_t pos, const json & jConfig);
    static bool editAction(const json & jConfig);

    static bool ruleUp(uint32_t pos);
    static bool ruleDown(uint32_t pos);


    static bool addRule(uint32_t pos, const json & jConfig );
    static bool addAction(const json &jConfig);

    static bool removeRule(const uint32_t & pos);
    static bool removeAction(const std::string &actionName);

    static void setRulesFilePath(const std::string &value);
    static void setActionsFilePath(const std::string &value);

    static bool writeRulesFileConfig(const json & jConfig);
    static bool writeActionsFileConfig(const json & jConfig);

    static json readRulesFileConfig();
    static json readActionsFileConfig();

    static json getCurrentRunningRule(const uint32_t &pos);
    static json getCurrentRunningRules();
    static json getCurrentRunningActions();
    static json getCurrentRunningAction(const std::string & sActionName);

    static bool getRulesModified();
    static bool getActionsModified();

    static bool reloadRules();
    static bool reloadActions();


private:

    ////////////////////////////////////////////////////////////////////////////////
    // Config Rules/Actions:
    static bool _addRule(uint32_t pos, const json &jConfig, bool replace=false );
    static bool _addAction(const json &jConfig, bool replace=false);

    static bool _removeRule( const uint32_t & pos );
    static bool _removeAction( const std::string & actionName );

    static void threadEvaluation(const uint32_t & threadId);
    static void threadPassCTStatsEverySecond();

    static void exec(const std::string &ruleName, const char * file, std::vector<std::string> vArguments,const json &values);
    static void replaceArgumentsVars(std::vector<std::string> & vArguments,const std::string &ruleName, const json &values);
    static void replaceArgumentsVar(std::string & vArgument,const std::string &ruleName, const json &values);
    static std::string getValueForVar(const std::string &var, const std::string &ruleName, const json &values);


    static void resetRules();
    static void resetActions();

    static std::string getRandomTag(const std::string & str, uint64_t t);

    static uint64_t maxQueuePushWaitTimeInMilliseconds, maxQueuePopTimeInMilliseconds;
    static CX2::Threads::Safe::Queue<std::string> evaluationQueue;
    static CX2::Threads::Sync::Mutex_Shared mtRules;
    static std::vector<sRule *> rules;
    static std::map<std::string,sAction *> actions;

    static std::atomic<uint64_t> queuedLinesCount, droppedInQueue;
    static std::atomic<uint64_t> badFormatLines, processedLines;
    static std::atomic<uint64_t> rulesActivated;
    static std::atomic<uint64_t> queueInLastSec,queueOutLastSec;
    static std::atomic<uint64_t> queueInLastSec_ct,queueOutLastSec_ct;
    static std::atomic<double> evaluationTimeInMS;

    //static json currentRunningRules, currentRunningActions;
    static std::string sRulesFilePath,sActionsFilePath;
    static bool bRulesModified,bActionsModified;

};

}}}

#endif // RULEACTION_H

