#include "audit_host.h"
#include "globals_ext.h"

#include "output/processorthreads_output.h"

#include <set>

using namespace std;


Audit_Host::Audit_Host()
{
}

Audit_Host::~Audit_Host()
{
    for (auto & i : auditEvents)
    {
        delete i.second;
    }
}

void Audit_Host::insertClassContents(const std::tuple<time_t, uint32_t, uint64_t> &eventId, const std::string &eventType, std::string * varData)
{

    mEvents.lock();
    Audit_Event * aevent = nullptr;

    if ( auditEvents.find(eventId) != auditEvents.end() )
        aevent = auditEvents[eventId];
    else
    {
        aevent = new Audit_Event;
        auditEvents[eventId] = aevent;
        aevent->setEventId(eventId);
        aevent->setHostName(hostname);
    }

    if (aevent->insertClassContents(eventType,varData))
    {
        // Event reception ready/completed for analysis, take it out from here and put it in the queue!.
        auditEvents.erase(eventId);

        mEvents.unlock();
        if (!ProcessorThreads_Output::pushAuditEvent(aevent))
        {
            std::tuple<time_t, uint32_t, uint64_t> eventId = aevent->getEventId();
            SERVERAPP->getLogger()->error("Queue full, Event %u.%u:%u Dropped...", (uint32_t) get<0>(eventId),get<1>(eventId),(uint32_t)get<2>(eventId) );
            delete aevent;
        }
    }
    else
    {
        mEvents.unlock();
    }
}

void Audit_Host::setHostname(const std::string &value)
{
    if (hostname.empty()) hostname = value;
}

void Audit_Host::dropOldUncompletedEvents(const uint64_t &maxEventTimeInSeconds)
{
    mEvents.lock();

    set<tuple<time_t,uint32_t,uint64_t>> uncompleted_phase1;
    set<Audit_Event *> uncompleted_phase2;

    for (auto & i : auditEvents)
    {
        if (i.second->timeSinceCreation()>maxEventTimeInSeconds)
        {
            uncompleted_phase1.insert(i.first);
            uncompleted_phase2.insert(i.second);
        }
    }

    for (auto & eventId : uncompleted_phase1)
    {
        auditEvents.erase(eventId);
    }

    mEvents.unlock();

    string uncompletedEventIDs = "";
    for (Audit_Event * aevent : uncompleted_phase2)
    {
        char cEventID[512];
        std::tuple<time_t, uint32_t, uint64_t> eventId = aevent->getEventId();
        snprintf(cEventID,500,"%ld.%u:%lu,", get<0>(eventId),get<1>(eventId),get<2>(eventId));
        uncompletedEventIDs+=cEventID;
    }

    if (!uncompletedEventIDs.empty())
    {
        uncompletedEventIDs.pop_back();
        SERVERAPP->getLogger()->error("Queueing uncompleted events for host='%s' (%s)...", this->hostname, uncompletedEventIDs );
    }

    for (Audit_Event * aevent : uncompleted_phase2)
    {
        if (!ProcessorThreads_Output::pushAuditEvent(aevent))
        {
            char cEventID[512];
            std::tuple<time_t, uint32_t, uint64_t> eventId = aevent->getEventId();
            snprintf(cEventID,500,"%ld.%u:%lu", get<0>(eventId),get<1>(eventId),get<2>(eventId));
            SERVERAPP->getLogger()->error("Queue full, Event %s Dropped...", string(cEventID) );
            delete aevent;
        }
    }

}
