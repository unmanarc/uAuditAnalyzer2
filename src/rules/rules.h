#ifndef RULEACTION_H
#define RULEACTION_H

#include <list>
#include <string>
#include <mutex>

#include <boost/property_tree/ptree.hpp>

#include <json/json.h>

#include <cx_scripts_jsonexpreval/jsonexpreval.h>

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
    JSONExprEval *expr;
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

std::string toUnStyledString(const Json::Value& value);

class Rules
{
public:
    Rules();
    ~Rules();

    static void startExecFork();
    static bool reloadRules(const std::string &dirPath);
    static bool reloadActions(const std::string &dirPath);
    static bool evaluate( const Json::Value & values );

private:

    static void exec(const std::string &ruleName, const char * file, std::vector<std::string> vArguments,const Json::Value &values);
    static void replaceArgumentsVars(std::vector<std::string> & vArguments,const std::string &ruleName, const Json::Value &values);
    static void replaceArgumentsVar(std::string & vArgument,const std::string &ruleName, const Json::Value &values);
    static std::string getValueForVar(const std::string &var, const std::string &ruleName, const Json::Value &values);

    static void addNewRule(const std::string & ruleName, const boost::property_tree::ptree &vars );
    static void addNewAction(const std::string & actionName, const boost::property_tree::ptree &vars );

    static void resetRules();
    static void resetActions();

    static std::mutex mtRules;
    static std::list<sRule *> rules;
    static std::map<std::string,sAction *> actions;
};

#endif // RULEACTION_H

