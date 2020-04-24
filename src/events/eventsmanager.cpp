#include "eventsmanager.h"
#include "globals_ext.h"

#include <mutex>
#include <thread>


std::map<auditHostID,Audit_Host> EventsManager::eventsByHostName;
std::mutex EventsManager::mutex_insert;


void garbageCollector()
{
    std::string threadName = "EVENTS_GC";
    pthread_setname_np(pthread_self(), threadName.c_str());
    EventsManager::gc();
}

EventsManager::EventsManager()
{

}

void EventsManager::writeStats(const string &outputDir)
{
    ofstream myfile;
    myfile.open (outputDir + "/auditd.events.count");
    myfile << "# Auditd events count per host\n";

    mutex_insert.lock();
    for ( auto & i : eventsByHostName )
    {
        myfile << "HOSTNAME=" << i.first.hostname << "\tIP=" << i.first.ip << "\tPENDING=" << i.second.getPendingEventsCount( ) << "\tPROCESSED=" << i.second.getCountEventsProcessed() << "\tDROPPED=" << i.second.getCountEventsDropped() << endl;
    }
    mutex_insert.unlock();

    myfile.close();
}

void EventsManager::gc()
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

void EventsManager::startGC()
{
    std::thread(garbageCollector).detach();
}

void EventsManager::insertClassContents(const auditHostID & hostID, const std::tuple<time_t, uint32_t, uint64_t> &eventId, const std::string &eventType, std::string *vardata )
{
    mutex_insert.lock();
    eventsByHostName[hostID].setHostID(hostID);
    eventsByHostName[hostID].insertClassContents(eventId,eventType,vardata);
    mutex_insert.unlock();
}
