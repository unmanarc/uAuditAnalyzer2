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
        Audit_ClassType * avars = new Audit_ClassType;
        avars->setRawInput(varData);
        avars->setClassTypeName(eventType);
        classVars.insert( make_pair( eventType, avars ) );
    }
    else
    {
        Audit_ClassType * avars = classVars.find(eventType)->second;
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

auditHostID Audit_Event::getHostId() const
{
    return hostId;
}

void Audit_Event::setHostId(const auditHostID &value)
{
    hostId = value;
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

std::list<Audit_ClassType *> Audit_Event::getMultiLineClassVars(const string &groupName)
{
    list<Audit_ClassType *> r;
    for (const auto & i : classVars)
    {
        if (i.first == groupName) r.push_back(i.second);
    }
    return r;
}


Audit_ClassType *Audit_Event::getClassVars(const string &groupName)
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

    std::string auditdComposedID =  to_string(unixTime) + "." + to_string(std::get<1>(eventId)) + ":" + to_string(std::get<2>(eventId));

    x["AUDITD"]["composedId"]  = auditdComposedID;

    x["INFO"]["parserVersion"]  = 1;

    x["INFO"]["unixTime"]  = (Json::Value::Int64)unixTime;
    x["INFO"]["msecs"]     = std::get<1>(eventId);
    x["INFO"]["id"]        = (Json::Value::UInt64)std::get<2>(eventId);

    x["INFO"]["host"]      = hostId.hostname;
    x["INFO"]["ip"]        = hostId.ip;
    x["INFO"]["time"]      = sTime;

    std::set<string> classes = getClassesNames();
    for (const string & className : classes)
    {
        int i=0;
        typedef std::multimap<std::string, Audit_ClassType *>::iterator MMAPIterator;
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
    }
    return x;
}

