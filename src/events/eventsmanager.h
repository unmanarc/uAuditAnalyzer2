#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include "audit_host.h"

#include <string>
#include <atomic>
#include <map>
#include <thread>


class EventsManager
{
public:
    EventsManager();
    ~EventsManager();

    void gc();

    void insertClassContents(const std::string & hostName, // hostname
                           const std::tuple<time_t,uint32_t,uint64_t> & eventId, // event time -> event msecs -> event id
                           const std::string & eventType,
                           std::string *vardata);

    bool getFinished();

private:
    std::map<std::string,Audit_Host> eventsByHostName;
    std::thread t;
    std::atomic<bool> finished;

};

#endif // EVENTSMANAGER_H
