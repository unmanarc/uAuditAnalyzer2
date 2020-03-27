#ifndef HOSTEVENTSCONTAINER_H
#define HOSTEVENTSCONTAINER_H

#include <string>
#include <mutex>
#include <map>

#include "audit_event.h"

class Audit_Host
{
public:
    Audit_Host();
    ~Audit_Host();

    void insertClassContents(const std::tuple<time_t,uint32_t,uint64_t> & eventId, // event time -> event msecs -> event id
                             const std::string & eventType,
                             std::string *varData);

    void setHostname(const std::string &value);

    void dropOldUncompletedEvents( const uint64_t & maxEventTimeInSeconds );


private:
    std::mutex mEvents;
    std::map< std::tuple<time_t,uint32_t,uint64_t>, Audit_Event * > auditEvents;
    std::string hostname;
};

#endif // HOSTEVENTSCONTAINER_H
