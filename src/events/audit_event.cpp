#include "audit_event.h"
#include "vars/namedefs.h"

#include <ctime>
#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
using namespace boost::algorithm;


using namespace std;

Audit_Event::Audit_Event()
{
    creationTime = time(nullptr);
}

Audit_Event::~Audit_Event()
{
    for (auto & i : classVars)
    {
        delete i.second;
    }
}

uint64_t Audit_Event::timeSinceCreation()
{
    return time(nullptr)-creationTime;
}

bool Audit_Event::insertClassContents(const string &eventType, std::string * varData)
{
    if (eventType == "PATH" || classVars.find(eventType) == classVars.end() )
    {
        // This is multi...
        Audit_Class * avars = new Audit_Class;
        avars->setRawInput(varData);
        avars->setClassName(eventType);
        classVars.insert( make_pair( eventType, avars ) );
    }
    else
    {
        Audit_Class * avars = classVars.find(eventType)->second;
        avars->getRawInput()->append(" ");
        avars->getRawInput()->append(*varData);
        delete varData;
    }

    return NameDefs::isEndEventType(eventType);
}

std::tuple<time_t, uint32_t, uint64_t> Audit_Event::getEventId() const
{
    return eventId;
}

void Audit_Event::setEventId(const std::tuple<time_t, uint32_t, uint64_t> &value)
{
    eventId = value;
}

std::string Audit_Event::getHostName() const
{
    return hostName;
}

void Audit_Event::setHostName(const std::string &value)
{
    hostName = value;
}

bool Audit_Event::process()
{
    for (auto & i : classVars)
    {
        i.second->processToVarContent();
    }

    // TODO: classVars -> sql string?
    return true;
}

std::set<string> Audit_Event::getClassesNames()
{
    std::set<string> r;
    for (auto & i : classVars)
        r.insert(i.first);
    return r;

}

bool Audit_Event::isMultiContainer(const string &groupName)
{
    auto result = classVars.equal_range(groupName);
    return std::distance(result.first, result.second) > 1;
}

std::list<Audit_Class *> Audit_Event::getMultiLineClassVars(const string &groupName)
{
    list<Audit_Class *> r;
    for (const auto & i : classVars)
    {
        if (i.first == groupName) r.push_back(i.second);
    }
    return r;
}


Audit_Class *Audit_Event::getClassVars(const string &groupName)
{
    if (classVars.find(groupName) != classVars.end())
    {
        return classVars.find(groupName)->second;
    }
    return nullptr;
}

Json::Value Audit_Event::getJSON()
{
    Json::Value x;

    time_t unixTime = std::get<0>(eventId);
    std::string  sTime = std::asctime(std::localtime(&unixTime));
    while (ends_with(sTime,"\n")) sTime.pop_back();

    x["INFO"]["unixTime"]  = (Json::Value::Int64)unixTime;
    x["INFO"]["msecs"]     = std::get<1>(eventId);
    x["INFO"]["id"]        = std::get<2>(eventId);
    x["INFO"]["host"]  = hostName;
    x["INFO"]["time"] = sTime;

    std::set<string> classes = getClassesNames();
    for (const string & className : classes)
    {
        /*    if (isMultiContainer(className))
        {*/
        int i=0;
        typedef std::multimap<std::string, Audit_Class *>::iterator MMAPIterator;
        std::pair<MMAPIterator, MMAPIterator> result = classVars.equal_range(className);
        for (MMAPIterator it = result.first; it != result.second; it++)
        {
            if (className != "INFO")
            {
                x[className][i] = it->second->getJSON();
                i++;
            }
            // TODO: limit max classes
        }
        if (i && x[className][0].isMember("key"))
        {
            x["INFO"]["key"] = x[className][0]["key"];
        }
        /*     }
        else
        {
            x["classes"][className]["multi"] = false;
            x["classes"][className]["items"][0] = classVars.find(className)->second->getJSON();
        }*/
    }
    return x;
}

