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
char ** Rules::envp = nullptr;
mutex Rules::mtRules;

void forkExec(const std::string & ruleName, const char *file, char ** envp, std::vector<string> vArguments )
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
        execvpe(file, argv, envp);
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
        reset();
    }
}

bool Rules::reload(const string &dirPath)
{
    const lock_guard<mutex> lock(mtRules);
    reset();

    if (!access(dirPath.c_str(),R_OK))
    {
#ifdef WIN32
        string envPath = dirPath + "\\environment.ini" ;
#else
        string envPath = dirPath + "/environment.ini" ;
#endif
        if (!access(envPath.c_str(),R_OK))
        {
            // Load Environment
            SERVERAPP->getLogger()->information("Loading environment vars on: %s", envPath);

            property_tree::ptree env_filters;
            property_tree::ini_parser::read_ini( envPath.c_str(),env_filters);
            property_tree::ptree vars = env_filters.get_child("Environment");

            envp = (char **)malloc( (vars.size()+1) * sizeof(char *)  );
            memset(envp,0,(vars.size()+1) * sizeof(char *));

            size_t i=0;
            for ( auto & var : vars )
            {
                string envVar = var.first + "=" + vars.get<string>(var.first,"");
                envp[i] = strdup( envVar.c_str() );
                i++;
            }
        }

        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0 && string(ent->d_name)!="environment.ini")
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
                        //                        SERVERAPP->getLogger()->information("Loading filter: %s", i.first);
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
                SERVERAPP->getLogger()->debug("Rule '%s' activated, executing %s",rule->name, string(rule->file));
                exec(rule->name,rule->file,rule->vArguments,values);
                if (rule->ruleAction == RULE_ACTION_EXECANDABORT)
                    break;
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
    forkExec(ruleName,file,envp,vArguments);
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
    if (vars.get<string>("Action", "") == "EXEC" && vars.get<string>("ActionBin", "") == "")
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'ActionBin' required variable is missing, aborting.",ruleName);
        return;
    }
    if (vars.get<string>("Action", "") == "EXEC" && vars.get<string>("ActionArgument[0]", "") == "")
    {
        SERVERAPP->getLogger()->warning("Rule '%s': 'ActionArgument[0]' required variable is missing, aborting.",ruleName);
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
        rule->setFileName(vars.get<string>("ActionBin", ""));
        vector<string> arguments;
        for (size_t i=0;vars.get<string>(string("ActionArgument[") + to_string(i) + "]", "") != "";i++)
        {
            //AppendNewLineToEarchArgument
            std::string argument = vars.get<string>(string("ActionArgument[") + to_string(i) + "]", "");
            if (vars.get<bool>("AppendNewLineToEarchArgument", false)) argument+="%N%";
            arguments.push_back(argument);
        }
        rule->setArguments(arguments);
    }
    else if (vars.get<string>("Action", "") == "ABORT")
    {
        rule->ruleAction = RULE_ACTION_ABORT;
    }


    rules.push_back(rule);
}

void Rules::setEnvironment(const vector<string> &vEnvVars)
{
    const lock_guard<mutex> lock(mtRules);
    resetEnvironment();
    envp = (char **)malloc( (vEnvVars.size() + 1)*sizeof(char *) );
    for (size_t i=0; i<vEnvVars.size();i++) envp[i] = (char *)strdup(vEnvVars[i].c_str());
    envp[vEnvVars.size()] = 0;
}

void Rules::reset()
{
    resetRules();
    resetEnvironment();
}

void Rules::resetEnvironment()
{
    if (envp)
    {
        for (int i=0; envp[i]; i++) free(envp[i]);
        free(envp);
        envp=nullptr;
    }
}

void Rules::resetRules()
{
    for (sRule * rule: rules) delete rule;
    rules.clear();
}
