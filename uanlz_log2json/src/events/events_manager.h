#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include "audit_host.h"

#include <string>
#include <atomic>
#include <map>

namespace UANLZ { namespace LOG2JSON { namespace AuditdEvents {

class Events_Manager
{
public:
    Events_Manager();
    ~Events_Manager();


    static json getJSONStats();

    static void gc();
    static void startGC();

    static void insertClassContents(const auditHostID & hostID,  // hostname/ip
                           const std::tuple<time_t,uint32_t,uint64_t> & eventId, // event time -> event msecs -> event id
                           const std::string & eventType,
                           std::string *vardata);

    static bool getFinished();

private:
    static std::map<auditHostID,Audit_Host> eventsByHostName;
    static std::mutex mutex_insert;
};

}}}

#endif // EVENTSMANAGER_H
