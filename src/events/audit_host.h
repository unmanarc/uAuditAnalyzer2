#ifndef HOSTEVENTSCONTAINER_H
#define HOSTEVENTSCONTAINER_H

#include <string>
#include <mutex>
#include <map>
#include <atomic>

#include "audit_event.h"


class Audit_Host
{
public:
    Audit_Host();
    ~Audit_Host();

    void insertClassContents(const std::tuple<time_t,uint32_t,uint64_t> & eventId, // event time -> event msecs -> event id
                             const std::string & eventType,
                             std::string *varData);

    void setHostID(const auditHostID &value);

    void dropOldUncompletedEvents( const uint64_t & maxEventTimeInSeconds );

    size_t getPendingEventsCount();
    uint64_t getCountEventsDropped() const;
    uint64_t getCountEventsProcessed() const;

private:
    std::mutex mEvents;
    std::map< std::tuple<time_t,uint32_t,uint64_t>, Audit_Event * > auditEvents;
    auditHostID hostid;
    std::atomic<uint64_t> countEventsDropped, countEventsProcessed;
};

#endif // HOSTEVENTSCONTAINER_H
