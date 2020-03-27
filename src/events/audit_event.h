#ifndef EVENT_CONTAINER_H
#define EVENT_CONTAINER_H


#include <list>
#include "audit_class.h"

class Audit_Event
{
public:
    Audit_Event();
    ~Audit_Event();

    uint64_t timeSinceCreation();

    bool insertClassContents(const std::string & eventType, std::string * varData);

    std::tuple<time_t, uint32_t, uint64_t> getEventId() const;
    void setEventId(const std::tuple<time_t, uint32_t, uint64_t> &value);

    std::string getHostName() const;
    void setHostName(const std::string &value);

    bool process();

    std::set<std::string> getClassesNames();
    bool isMultiContainer(const std::string & groupName);
    std::list<Audit_Class *> getMultiLineClassVars(const std::string & groupName);
    Audit_Class * getClassVars(const std::string & groupName);

    Json::Value getJSON();

private:
    // eventType -> ClassEvent
    std::multimap<std::string, Audit_Class *> classVars;
    std::tuple<time_t, uint32_t, uint64_t> eventId;
    std::string hostName;
    time_t creationTime;

};

#endif // EVENT_CONTAINER_H
