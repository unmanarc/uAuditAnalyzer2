#ifndef RULEACTION_H
#define RULEACTION_H

#include <list>
#include <string>
#include <mutex>

#include <cx2_scripts_jsonexpreval/jsoneval.h>
#include <cx2_thr_mutex/mutex_shared.h>
#include <cx2_thr_safecontainers/queue.h>

#include <boost/property_tree/ptree.hpp>
#include <json/json.h>

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
};

class Rules
{
public:
    Rules();
    ~Rules();

    static Json::Value getStats();

    static bool pushElementOnEvaluationQueue( const std::string & str );

    static bool startEvaluationThreads( const uint32_t & threadsCount );
    static void setEvaluationQueueMaxSize( const size_t & max );
    static void setMaxQueuePushWaitTimeInMilliseconds(const uint64_t &value);
    static void setMaxQueuePopTimeInMilliseconds(const uint64_t &value);


    static bool reloadRules();
    static bool reloadActions();

    static bool reloadRules(const std::string &dirPath);
    static bool reloadActions(const std::string &dirPath);

    static bool evaluate( const Json::Value & values );


private:
    static void threadEvaluation(const uint32_t & threadId);
    static void threadPassCTStatsEverySecond();

    static void exec(const std::string &ruleName, const char * file, std::vector<std::string> vArguments,const Json::Value &values);
    static void replaceArgumentsVars(std::vector<std::string> & vArguments,const std::string &ruleName, const Json::Value &values);
    static void replaceArgumentsVar(std::string & vArgument,const std::string &ruleName, const Json::Value &values);
    static std::string getValueForVar(const std::string &var, const std::string &ruleName, const Json::Value &values);

    static void addNewRule(const std::string & ruleName, const boost::property_tree::ptree &vars );
    static void addNewAction(const std::string & actionName, const boost::property_tree::ptree &vars );

    static void resetRules();
    static void resetActions();

    static std::string getRandomTag(const std::string & str, uint64_t t);

    static uint64_t maxQueuePushWaitTimeInMilliseconds, maxQueuePopTimeInMilliseconds;
    static CX2::Threads::Safe::Queue<std::string> evaluationQueue;
    static CX2::Threads::Sync::Mutex_Shared mtRules;
    static std::list<sRule *> rules;
    static std::map<std::string,sAction *> actions;

    static std::atomic<uint64_t> queuedLinesCount, droppedInQueue;
    static std::atomic<uint64_t> badFormatLines, processedLines;
    static std::atomic<uint64_t> rulesActivated;
    static std::atomic<uint64_t> queueInLastSec,queueOutLastSec;
    static std::atomic<uint64_t> queueInLastSec_ct,queueOutLastSec_ct;
    static std::atomic<double> evaluationTimeInMS;


    static std::string lastRulesDirPath, lastActionsDirPath;

};

}}}

#endif // RULEACTION_H

