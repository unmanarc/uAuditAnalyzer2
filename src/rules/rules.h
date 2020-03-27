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
    RULE_ACTION_UNDEFINED
};

struct sRule
{
    sRule()
    {
        ruleAction = RULE_ACTION_UNDEFINED;
        expr = nullptr;
        file = nullptr;
    }
    ~sRule()
    {
        if (expr) delete expr;
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

    eRuleAction ruleAction;
    std::string name;
    std::string filter;
    char * file;
    std::vector<std::string> vArguments;
    JSONExprEval *expr;
};

std::string toUnStyledString(const Json::Value& value);

class Rules
{
public:
    Rules();
    ~Rules();

    static void startExecFork();
    static bool reload(const std::string &dirPath);
    static bool evaluate( const Json::Value & values );

private:

    static void exec(const std::string &ruleName, const char * file, std::vector<std::string> vArguments,const Json::Value &values);
    static void replaceArgumentsVars(std::vector<std::string> & vArguments,const std::string &ruleName, const Json::Value &values);
    static void replaceArgumentsVar(std::string & vArgument,const std::string &ruleName, const Json::Value &values);
    static std::string getValueForVar(const std::string &var, const std::string &ruleName, const Json::Value &values);



    static void addNewRule(const std::string & ruleName, const boost::property_tree::ptree &vars );
    static void setEnvironment(const std::vector<std::string> & vEnvVars);

    static void reset();
    static void resetEnvironment();
    static void resetRules();

    static std::mutex mtRules;
    static char ** envp;
    static std::list<sRule *> rules;
};

#endif // RULEACTION_H
