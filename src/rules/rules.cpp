#include "rules.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/case_conv.hpp>


#include "globals_ext.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <random>
#include <string>

using namespace std;
using namespace boost;

list<sRule *> Rules::rules;
std::map<std::string,sAction *> Rules::actions;
mutex Rules::mtRules;

void forkExec(const std::string & ruleName, const char *file, std::vector<string> vArguments )
{
    char ** argv = (char **)malloc( (vArguments.size()+1)*sizeof(char *) );
    for (size_t i=0; i<vArguments.size();i++)
    {
        argv[i] = (char *)strdup(vArguments[i].c_str());
    }
    argv[vArguments.size()] = 0;

    //pid_t parent = getpid();
    pid_t pid = fork();

    if (pid == -1)
    {
        // error, failed to fork()
    }
    else if (pid > 0)
    {
        // Me...
        int status;
        waitpid(pid, &status, 0);
        if (status!=0)
        {
            SERVERAPP->getLogger()->warning("Rule '%s': execution of '%s' failed with %d (%s).",ruleName, string(file), status, string(strerror(errno)));
        }
    }
    else
    {
        // the child... (process is replaced here)
        execvp(file, argv);
        _exit(EXIT_FAILURE);   // exec never returns
    }

    for (int i=0; argv[i]; i++) free(argv[i]);
    free(argv);
}


std::string createRandomString16()
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);
    return str.substr(0, 16);
}


std::string getRandomTag(const std::string & str, uint64_t t)
{
    return "__" + str + "[" + std::to_string(t) + "]";
}


Rules::Rules()
{

}

Rules::~Rules()
{
    {
        const lock_guard<mutex> lock(mtRules);
        resetRules();
        resetActions();
    }
}

bool Rules::reloadRules(const string &dirPath)
{
    const lock_guard<mutex> lock(mtRules);
    resetRules();

    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                {
                    property_tree::ptree filters;
#ifdef WIN32
                    string filterFilePath = dirPath + "\\" + ent->d_name;
#else
                    string filterFilePath = dirPath + "/" + ent->d_name;
#endif
                    SERVERAPP->getLogger()->information("Loading filters from file: %s", filterFilePath);

                    property_tree::ini_parser::read_ini( filterFilePath, filters );

                    for ( auto & i : filters)
                    {
                        addNewRule(i.first,filters.get_child(i.first));
                    }
                }
            }
            closedir (dir);
        }
        else
        {
            SERVERAPP->getLogger()->error("Failed to list directory: %s", dirPath);
        }
        return true;
    }
    else
    {
        SERVERAPP->getLogger()->critical("Missing/Unreadable filters directory: %s", dirPath);
        return false;
    }
}

bool Rules::reloadActions(const string &dirPath)
{
    const lock_guard<mutex> lock(mtRules);
    resetActions();

    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                {
                    property_tree::ptree filters;
#ifdef WIN32
                    string filterFilePath = dirPath + "\\" + ent->d_name;
#else
                    string filterFilePath = dirPath + "/" + ent->d_name;
#endif
                    SERVERAPP->getLogger()->information("Loading actions from file: %s", filterFilePath);

                    property_tree::ini_parser::read_ini( filterFilePath, filters );

                    for ( auto & i : filters)
                    {
                        addNewAction(i.first,filters.get_child(i.first));
                    }
                }
            }
            closedir (dir);
        }
        else
        {
            SERVERAPP->getLogger()->error("Failed to list directory: %s", dirPath);
        }
        return true;
    }
    else
    {
        SERVERAPP->getLogger()->critical("Missing/Unreadable filters directory: %s", dirPath);
        return false;
    }
}

bool Rules::evaluate(const Json::Value &values)
{
    const lock_guard<mutex> lock(mtRules);

    bool activated = false;
    for (sRule * rule: rules)
    {
        if (rule->expr->evaluate(values))
        {
            // Activate
            activated = true;

            if (rule->ruleAction == RULE_ACTION_EXEC || rule->ruleAction == RULE_ACTION_EXECANDABORT)
            {
                if (actions.find(rule->actionId) == actions.end())
                {
                    SERVERAPP->getLogger()->error("Rule '%s' activated, but action '%s' not found",rule->name, rule->actionId);
                }
                else
                {
                    SERVERAPP->getLogger()->debug("Rule '%s' activated, executing action '%s'",rule->name, rule->actionId);
                    exec(rule->name, actions[rule->actionId]->file,  actions[rule->actionId]->vArguments,values);
                    if (rule->ruleAction == RULE_ACTION_EXECANDABORT) break;
                }
            }
            else if (rule->ruleAction == RULE_ACTION_ABORT)
            {
                SERVERAPP->getLogger()->debug("Rule '%s' activated, aborting next rules...",rule->name);
                break;
            }
        }
    }
    return activated;
}

void Rules::exec(const std::string & ruleName, const char *file, std::vector<string> vArguments,const Json::Value &values )
{
    replaceArgumentsVars(vArguments,ruleName,values);
    forkExec(ruleName,file,vArguments);
}

void Rules::replaceArgumentsVars(std::vector<string> &vArguments,const std::string &ruleName,const Json::Value &values)
{
    size_t i=0;
    for ( std::string & vArgument : vArguments)
    {
        if (i!=0)
            replaceArgumentsVar(vArgument,ruleName,values);
        i++;
    }
}

void Rules::replaceArgumentsVar(string &vArgument, const std::string &ruleName, const Json::Value &values)
{
    boost::match_flag_type flags = boost::match_default;
    // PRECOMPILE _STATIC_TEXT
    std::string secureTag = createRandomString16();
    uint64_t tagCount=0;
    std::map<std::string,std::string> intermediateTags;

    boost::regex exVar("(?<STATIC_TEXT>\\%[^\\%]*\\%)");
    boost::match_results<string::const_iterator> whatStaticText;
    for (string::const_iterator start = vArgument.begin(), end =  vArgument.end();
         boost::regex_search(start, end, whatStaticText, exVar, flags);
         start = vArgument.begin(), end =  vArgument.end())
    {
        std::string xValue = string(whatStaticText[1].first, whatStaticText[1].second).substr(1);
        xValue.pop_back();
        boost::replace_all(vArgument, string(whatStaticText[1].first, whatStaticText[1].second), getRandomTag(secureTag,tagCount) );
        intermediateTags[ getRandomTag(secureTag,tagCount++) ] = xValue;
    }
    for (const auto & repl : intermediateTags)
    {
        boost::replace_all(vArgument, repl.first, getValueForVar(repl.second,ruleName,values));
    }
}

string Rules::getValueForVar(const string &var, const string &ruleName, const Json::Value &values)
{
    if (var.empty()) return "%";

    else if (var == "N")
    {
        return "\n";
    }
    else if (var.at(0) == '$')
    {
        // JSONPATH MULTI-LINE STYLED
        Json::Path path(var.substr(1));
        Json::Value result = path.resolve(values);
        return result.toStyledString();
    }
    else if (var.at(0) == '-')
    {
        // JSONPATH ONE-LINE UNSTYLED
        Json::Path path(var.substr(1));
        Json::Value result = path.resolve(values);
        return toUnStyledString(result);
    }
    else if (var.at(0) == '+')
    {
        // JSONPATH ONE-LINE UNSTYLED MAYUS
        Json::Path path(var.substr(1));
        Json::Value result = path.resolve(values);
        return boost::to_upper_copy(toUnStyledString(result));
    }
    else if (var.at(0) == '#')
    {
        if (var.substr(1) == "rule.name")
        {
            return ruleName;
        }
        SERVERAPP->getLogger()->debug("Invalid Argument Variable '%s' on rule '%s'", var,ruleName);
        return "INVALID_VARGUMENT_VARIABLE";
    }
    else
    {
        SERVERAPP->getLogger()->debug("Invalid Argument Variable '%s' on rule '%s'", var,ruleName);
        return "INVALID_VARGUMENT_VARIABLE";
    }

}

void Rules::addNewRule(const string &ruleName, const property_tree::ptree &vars)
{
    if (vars.get<string>("Filter", "") == "")
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'Filter' required variable is missing, aborting.",ruleName);
        return;
    }
    if (vars.get<string>("Action", "") == "")
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'Action' required variable is missing, aborting.",ruleName);
        return;
    }
    if (vars.get<string>("Action", "") == "EXEC" && vars.get<string>("ActionID", "") == "")
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'ActionID' required variable is missing, aborting.",ruleName);
        return;
    }
    SERVERAPP->getLogger()->information("Creating Rule '%s'",ruleName);

    sRule * rule = new sRule;
    rule->name = ruleName;
    rule->expr = new JSONExprEval(vars.get<string>("Filter", ""));

    if (!rule->expr->getIsCompiled())
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'Filter' compilation failed with '%s'.",ruleName, rule->expr->getLastCompilerError());
        delete rule;
        return;
    }

    if (vars.get<string>("Action", "") == "EXEC" || vars.get<string>("Action", "") == "EXECANDABORT")
    {
        rule->ruleAction =  vars.get<string>("Action", "") == "EXEC"? RULE_ACTION_EXEC : RULE_ACTION_EXECANDABORT;
        rule->actionId = vars.get<string>("ActionID", "");
    }
    else if (vars.get<string>("Action", "") == "ABORT")
    {
        rule->ruleAction = RULE_ACTION_ABORT;
    }


    rules.push_back(rule);
}

void Rules::addNewAction(const string &actionName, const property_tree::ptree &vars)
{
    if (actions.find(actionName) == actions.end())
    {
        SERVERAPP->getLogger()->warning("Action '%s': duplicated name.",actionName);
        return;
    }

    if ( vars.get<string>("PathName", "") == "")
    {
        SERVERAPP->getLogger()->warning("Action '%s': 'PathName' required variable is missing, aborting.",actionName);
        return;
    }
    if (vars.get<string>("Argv[0]", "") == "")
    {
        SERVERAPP->getLogger()->warning("Action '%s': 'Argv[0]' required variable is missing, aborting.",actionName);
        return;
    }
    SERVERAPP->getLogger()->information("Creating Action '%s'",actionName);
    sAction * action = new sAction;
    action->setFileName(vars.get<string>("PathName", ""));
    vector<string> arguments;
    for (size_t i=0;vars.get<string>(string("Argv[") + to_string(i) + "]", "") != "";i++)
    {
        //AppendNewLineToEarchArgument
        std::string argument = vars.get<string>(string("Argv[") + to_string(i) + "]", "");
        if (vars.get<bool>("AppendNewLineToEarchArgument", false)) argument+="%N%";
        arguments.push_back(argument);
    }
    action->setArguments(arguments);
    actions[actionName] = action;
}


void Rules::resetRules()
{
    for (sRule * rule: rules) delete rule;
    rules.clear();
}

void Rules::resetActions()
{
    for (auto & i : actions) delete i.second;
    actions.clear();
}
