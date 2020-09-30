#include "rules.h"
#include <cx2_thr_mutex/lock_shared.h>
#include <cx2_hlp_functions/random.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "globals.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <random>
#include <string>
#include <thread>

using namespace std;
using namespace boost;

using namespace UANLZ::JSONALERT;
using namespace UANLZ::JSONALERT::Filters;

using namespace CX2::Application;
using namespace CX2::Scripts::Expressions;
using namespace CX2::Helpers;
using namespace CX2::Threads::Sync;

list<sRule *> Rules::rules;
std::map<std::string,sAction *> Rules::actions;
CX2::Threads::Sync::Mutex_Shared Rules::mtRules;
CX2::Threads::Safe::Queue<std::string> Rules::evaluationQueue;
uint64_t Rules::maxQueuePushWaitTimeInMilliseconds=0, Rules::maxQueuePopTimeInMilliseconds=10000;

std::atomic<uint64_t> Rules::queuedLinesCount, Rules::droppedInQueue;
std::atomic<uint64_t> Rules::badFormatLines, Rules::processedLines;
std::atomic<uint64_t> Rules::rulesActivated;
std::atomic<uint64_t> Rules::queueInLastSec,Rules::queueOutLastSec;
std::atomic<uint64_t> Rules::queueInLastSec_ct,Rules::queueOutLastSec_ct;

std::atomic<double> Rules::evaluationTimeInMS;

std::string Rules::lastRulesDirPath, Rules::lastActionsDirPath;


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
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Rule '%s': execution of '%s' failed with %d (%s).", ruleName.c_str(), string(file).c_str(), status, strerror(errno));
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

std::string toUnStyledString(const Json::Value& value)
{
    Json::FastWriter writer;
    std::string str = writer.write( value );
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

Rules::Rules()
{

}

Rules::~Rules()
{
    {
        Lock_RW lock(mtRules);
        resetRules();
        resetActions();
    }
}

Json::Value Rules::getStats()
{
    Json::Value j;
    j["queue"]["count"] = (Json::UInt64)evaluationQueue.size();
    j["queue"]["max"] = (Json::UInt64)evaluationQueue.getMaxItems();

    j["queuedLinesCount"]   = (Json::UInt64)queuedLinesCount;
    j["droppedInQueue"]     = (Json::UInt64)droppedInQueue;
    j["badFormatLines"]     = (Json::UInt64)badFormatLines;
    j["processedLines"]     = (Json::UInt64)processedLines;
    j["rulesActivated"]     = (Json::UInt64)rulesActivated;
    j["queueInLastSec"]     = (Json::UInt64)queueInLastSec;
    j["queueOutLastSec"]    = (Json::UInt64)queueOutLastSec;
    j["lastEvaluationTimeInMS"] = (double)evaluationTimeInMS;

    return j;
}

bool Rules::startEvaluationThreads(const uint32_t &threadsCount)
{
    if (threadsCount==0) return false;

    queuedLinesCount=0;
    droppedInQueue=0;
    badFormatLines=0;
    processedLines=0;
    rulesActivated=0;
    queueInLastSec=0;
    queueOutLastSec=0;
    queueInLastSec_ct=0;
    queueOutLastSec_ct=0;
    evaluationTimeInMS = 0;

    std::thread t0 = std::thread(threadPassCTStatsEverySecond);
    t0.detach();

    for (uint32_t i=0;i<threadsCount;i++)
    {
        std::thread t = std::thread(threadEvaluation,i+1);
        t.detach();
    }
    return true;
}

void Rules::setEvaluationQueueMaxSize(const size_t &max)
{
    evaluationQueue.setMaxItems(max);
}

bool Rules::pushElementOnEvaluationQueue(const string &str)
{
    std::string * strc = new std::string;
    *strc = str;
    if (!evaluationQueue.push(strc,maxQueuePushWaitTimeInMilliseconds))
    {
        delete strc;
        droppedInQueue++;
        return false;
    }
    queueInLastSec_ct++;
    queuedLinesCount++;
    return true;
}

bool Rules::reloadRules(const string &dirPath)
{
    Lock_RW lock(mtRules);
    resetRules();

    lastRulesDirPath = dirPath;

    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            std::set<std::string> files;

            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                {
                    files.insert(ent->d_name);

                }
            }
            closedir (dir);

            for (const std::string & file: files)
            {
                property_tree::ptree filters;
                string filterFilePath = dirPath + "/" + file;

                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,"Loading filters from file: %s", filterFilePath.c_str());

                property_tree::ini_parser::read_ini( filterFilePath, filters );

                for ( auto & i : filters)
                {
                    addNewRule(i.first,filters.get_child(i.first));
                }
            }

        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR,"Failed to list directory: %s, no rules loaded", dirPath.c_str());
        }
        return true;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL,"Missing/Unreadable filters directory: %s", dirPath.c_str());
        return false;
    }
}

bool Rules::reloadActions(const string &dirPath)
{
    Lock_RW lock(mtRules);
    resetActions();

    lastActionsDirPath = dirPath;

    if (!access(dirPath.c_str(),R_OK))
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath.c_str())) != NULL)
        {
            std::set<std::string> files;

            while ((ent = readdir (dir)) != NULL)
            {
                if ((ent->d_type & DT_REG) != 0)
                {
                    files.insert(ent->d_name);
                }
            }
            closedir (dir);

            for (const std::string & file: files)
            {
                property_tree::ptree filters;
                string filterFilePath = dirPath + "/" + file;

                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,"Loading actions from file: %s", filterFilePath.c_str());

                property_tree::ini_parser::read_ini( filterFilePath, filters );
                for ( auto & i : filters)
                {
                    addNewAction(i.first,filters.get_child(i.first));
                }
            }
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR,"Failed to list directory: %s, no actions loaded.", dirPath.c_str());
        }
        return true;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_CRITICAL,"Missing/Unreadable filters directory: %s", dirPath.c_str());
        return false;
    }
}

bool Rules::evaluate(const Json::Value &values)
{
    Lock_RD lock(mtRules);

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
                    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR,"Rule '%s' activated, but action '%s' not found", rule->name.c_str(), rule->actionId.c_str());
                }
                else
                {
                    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_DEBUG,"Rule '%s' activated, executing action '%s'", rule->name.c_str(), rule->actionId.c_str());
                    exec(rule->name, actions[rule->actionId]->file,  actions[rule->actionId]->vArguments,values);
                    if (rule->ruleAction == RULE_ACTION_EXECANDABORT) break;
                }
            }
            else if (rule->ruleAction == RULE_ACTION_ABORT)
            {
                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_DEBUG,"Rule '%s' activated, aborting next rules...", rule->name.c_str());
                break;
            }
        }
    }
    return activated;
}

void Rules::threadEvaluation(const uint32_t &threadId)
{
    std::string threadName = "JSONEVAL";
    pthread_setname_np(pthread_self(), threadName.c_str());

    Json::Value v;
    std::string * s;
    for (;;)
    {
        s = evaluationQueue.pop(maxQueuePopTimeInMilliseconds);
        queueOutLastSec_ct++;
        if (s)
        {
            auto start = chrono::high_resolution_clock::now();
            auto finish = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> elapsed = finish - start;

            Json::Reader reader;
            if (reader.parse(*s, v))
            {
                if (evaluate(v))
                {
                    rulesActivated++;
                }
                processedLines++;
            }
            else
            {
                Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_ERR,"Evaluation Thread #%d received invalid JSON...", threadId);
                badFormatLines++;
            }
            delete s;
            s=nullptr;

            finish = chrono::high_resolution_clock::now();
            elapsed = finish - start;
            evaluationTimeInMS = elapsed.count();
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_DEBUG,"Evaluation Thread #%d timed out (no elements)...", threadId);
        }
    }
}

void Rules::threadPassCTStatsEverySecond()
{
    std::string threadName = "COUNTERS";
    pthread_setname_np(pthread_self(), threadName.c_str());

    for (;;)
    {
        queueInLastSec = (uint64_t)queueInLastSec_ct;
        queueOutLastSec = (uint64_t)queueOutLastSec_ct;

        queueInLastSec_ct = 0;
        queueOutLastSec_ct = 0;

        sleep(1);
    }
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
    std::string secureTag = Random::createRandomString(16);
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
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_DEBUG,"Invalid Argument Variable '%s' on rule '%s'", var.c_str(),ruleName.c_str());
        return "INVALID_VARGUMENT_VARIABLE";
    }
    else
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_DEBUG,"Invalid Argument Variable '%s' on rule '%s'", var.c_str(),ruleName.c_str());
        return "INVALID_VARGUMENT_VARIABLE";
    }

}

void Rules::addNewRule(const string &ruleName, const property_tree::ptree &vars)
{
    if (vars.get<string>("Filter", "") == "")
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Rule '%s': 'Filter' required variable is missing, aborting.",ruleName.c_str());
        return;
    }
    if (vars.get<string>("Action", "") == "")
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Rule '%s': 'Action' required variable is missing, aborting.",ruleName.c_str());
        return;
    }
    if (vars.get<string>("Action", "") == "EXEC" && vars.get<string>("ActionID", "") == "")
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Rule '%s': 'ActionID' required variable is missing, aborting.",ruleName.c_str());
        return;
    }

    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,"Creating Rule '%s'",ruleName.c_str());

    sRule * rule = new sRule;
    rule->name = ruleName;
    rule->expr = new JSONEval(vars.get<string>("Filter", ""));

    if (!rule->expr->getIsCompiled())
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Rule '%s': 'Filter' compilation failed with '%s'.",ruleName.c_str(), rule->expr->getLastCompilerError().c_str());
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
    if (actions.find(actionName) != actions.end())
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Action '%s': duplicated name.",actionName.c_str());
        return;
    }

    if ( vars.get<string>("PathName", "") == "")
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Action '%s': 'PathName' required variable is missing, aborting.",actionName.c_str());
        return;
    }
    if (vars.get<string>("Argv[0]", "") == "")
    {
        Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_WARN,"Action '%s': 'Argv[0]' required variable is missing, aborting.",actionName.c_str());
        return;
    }

    Globals::getAppLog()->log0(__func__,Logs::LOG_LEVEL_INFO,"Creating Action '%s'",actionName.c_str());

    sAction * action = new sAction;
    action->setFileName(vars.get<string>("PathName", ""));
    vector<string> arguments;
    for (size_t i=0;vars.get<string>(string("Argv[") + to_string(i) + "]", "") != "";i++)
    {
        //AppendNewLineToEarchArgument
        std::string argument = vars.get<string>(string("Argv[") + to_string(i) + "]", "");
        if (vars.get<bool>("AppendNewLineToEarchArgv", false)) argument+="%N%";
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

string Rules::getRandomTag(const string &str, uint64_t t)
{
    return "__" + str + "[" + std::to_string(t) + "]";
}

void Rules::setMaxQueuePopTimeInMilliseconds(const uint64_t &value)
{
    maxQueuePopTimeInMilliseconds = value;
}

bool Rules::reloadRules()
{
    return reloadRules(lastRulesDirPath);
}

bool Rules::reloadActions()
{
    return reloadActions(lastActionsDirPath);
}

void Rules::setMaxQueuePushWaitTimeInMilliseconds(const uint64_t &value)
{
    maxQueuePushWaitTimeInMilliseconds = value;
}
