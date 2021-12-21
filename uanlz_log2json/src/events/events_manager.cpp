#include "events_manager.h"
#include "../globals.h"

#include <mutex>
#include <thread>

using namespace UANLZ::LOG2JSON::AuditdEvents;

std::map<auditHostID,Audit_Host> Events_Manager::eventsByHostName;
std::mutex Events_Manager::mutex_insert;

void garbageCollector()
{
    std::string threadName = "EVENTS_GC";
    pthread_setname_np(pthread_self(), threadName.c_str());
    Events_Manager::gc();
}

Events_Manager::Events_Manager()
{

}

json Events_Manager::getJSONStats()
{
    json v;
    mutex_insert.lock();
    int x=0;
    for ( auto & i : eventsByHostName )
    {
        v[x]["host"] = i.first.hostname;
        v[x]["ip"] = i.first.ip;
        v[x]["events"]["pending"] = (Json::UInt64) i.second.getPendingEventsCount( );
        v[x]["events"]["processed"] = (Json::UInt64) i.second.getCountEventsProcessed();
        v[x]["events"]["dropped"] = (Json::UInt64) i.second.getCountEventsDropped() ;
        x++;
    }
    mutex_insert.unlock();
    return v;
}

void Events_Manager::gc()
{
    for (;;)
    {
        uint64_t maxAuditWaitTime = Globals::getConfig_main()->get<uint64_t>("GC.AuditMaxAgeInSecs",10);
        uint64_t auditGCPeriod = Globals::getConfig_main()->get<uint64_t>("GC.GCPeriodInSecs",5);

        mutex_insert.lock();
        for ( auto & i : eventsByHostName )
        {
            i.second.dropOldUncompletedEvents( maxAuditWaitTime );
        }
        mutex_insert.unlock();

        sleep(auditGCPeriod);
    }
}

void Events_Manager::startGC()
{
    std::thread(garbageCollector).detach();
}

void Events_Manager::insertClassContents(const auditHostID & hostID, const std::tuple<time_t, uint32_t, uint64_t> &eventId, const std::string &eventType, std::string *vardata )
{
    mutex_insert.lock();
    eventsByHostName[hostID].setHostID(hostID);
    eventsByHostName[hostID].insertClassContents(eventId,eventType,vardata);
    mutex_insert.unlock();
}
