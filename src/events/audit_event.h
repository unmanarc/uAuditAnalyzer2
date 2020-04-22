#ifndef EVENT_CONTAINER_H
#define EVENT_CONTAINER_H


#include <list>
#include "audit_class.h"
struct auditHostID
{
    std::string hostname;
    std::string ip;
};

class Audit_Event
{
public:
    Audit_Event();
    ~Audit_Event();

    uint64_t timeSinceCreation();

    bool insertClassContents(const std::string & eventType, std::string * varData);

    std::tuple<time_t, uint32_t, uint64_t> getEventId() const;
    void setEventId(const std::tuple<time_t, uint32_t, uint64_t> &value);

    auditHostID getHostId() const;
    void setHostId(const auditHostID &value);

    bool process();

    std::set<std::string> getClassesNames();
    bool isMultiContainer(const std::string & groupName);
    std::list<Audit_ClassType *> getMultiLineClassVars(const std::string & groupName);
    Audit_ClassType * getClassVars(const std::string & groupName);

    Json::Value getJSON();

private:
    // eventType -> ClassEvent
    std::multimap<std::string, Audit_ClassType *> classVars;
    std::tuple<time_t, uint32_t, uint64_t> eventId;
    auditHostID hostId;
    time_t creationTime;

};

#endif // EVENT_CONTAINER_H
