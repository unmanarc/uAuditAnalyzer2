#include "eventsmanager.h"
#include "globals_ext.h"

void garbageCollector(EventsManager * ev)
{
    while (!ev->getFinished()) ev->gc();
}

EventsManager::EventsManager()
{
    finished = false;
    t = std::thread(garbageCollector,this);
}

EventsManager::~EventsManager()
{
    finished = true;
    t.join();
}

void EventsManager::gc()
{
    uint64_t maxAuditWaitTime = Globals::getConfig_main()->get<uint64_t>("GC.AuditMaxAgeInSecs",10);
    uint64_t auditGCPeriod = Globals::getConfig_main()->get<uint64_t>("GC.GCPeriodInSecs",5);

    for ( auto & i : eventsByHostName )
    {
        i.second.dropOldUncompletedEvents( maxAuditWaitTime );
    }

    sleep(auditGCPeriod);
}


void EventsManager::insertClassContents(const std::string &hostName, const std::tuple<time_t, uint32_t, uint64_t> &eventId, const std::string &eventType, std::string *vardata )
{
    eventsByHostName[hostName].setHostname(hostName);
    eventsByHostName[hostName].insertClassContents(eventId,eventType,vardata);
}

bool EventsManager::getFinished()
{
    return finished;
}
