#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include "audit_host.h"

#include <string>
#include <atomic>
#include <map>

class EventsManager
{
public:
    EventsManager();
    ~EventsManager();

    static void writeStats(const std::string & outputDir);

    static void gc();
    static void startGC();

    static void insertClassContents(const std::string & hostName, // hostname
                           const std::tuple<time_t,uint32_t,uint64_t> & eventId, // event time -> event msecs -> event id
                           const std::string & eventType,
                           std::string *vardata);

    static bool getFinished();

private:
    static std::map<std::string,Audit_Host> eventsByHostName;
    static std::mutex mutex_insert;

};

#endif // EVENTSMANAGER_H
