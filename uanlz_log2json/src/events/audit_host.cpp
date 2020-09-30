#include "audit_host.h"
#include "globals.h"

#include "events/events_distributionthreads.h"

#include <set>

using namespace std;
using namespace UANLZ::LOG2JSON::AuditdEvents;


Audit_Host::Audit_Host()
{
    countEventsDropped=0;
    countEventsProcessed=0;
    enforceMaxAge = Globals::getConfig_main()->get<uint64_t>("GC.EnforceMaxAge",true);
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
        aevent->setHostId(hostid);
    }

    if (aevent->insertClassContents(eventType,varData) && !enforceMaxAge)
    {
        // Event reception ready/completed for analysis, take it out from here and put it in the queue!.
        auditEvents.erase(eventId);
        mEvents.unlock();

        if (!Events_DistributionThreads::pushAuditEvent(aevent))
        {
            std::tuple<time_t, uint32_t, uint64_t> eventId = aevent->getEventId();
            Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LOG_LEVEL_ERR, "Queue full, Event %ld.%d:%lu Dropped...", get<0>(eventId),get<1>(eventId),get<2>(eventId) );
            delete aevent;
            countEventsDropped++;
        }
        else
        {
            countEventsProcessed++;
        }
    }
    else
    {
        mEvents.unlock();
    }
}

void Audit_Host::setHostID(const auditHostID &value)
{
    if (hostid.hostname.empty()) hostid = value;
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
        if (!enforceMaxAge) uncompletedEventIDs+=cEventID;
    }

    if (!uncompletedEventIDs.empty())
    {
        uncompletedEventIDs.pop_back();
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LOG_LEVEL_ERR, "Queueing uncompleted events for host='%s' (%s)...", this->hostid.hostname.c_str(), uncompletedEventIDs.c_str() );
    }

    for (Audit_Event * aevent : uncompleted_phase2)
    {
        if (!Events_DistributionThreads::pushAuditEvent(aevent))
        {
            char cEventID[512];
            std::tuple<time_t, uint32_t, uint64_t> eventId = aevent->getEventId();
            snprintf(cEventID,500,"%ld.%u:%lu", get<0>(eventId),get<1>(eventId),get<2>(eventId));
            Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LOG_LEVEL_ERR, "Queue full, Event %s Dropped...", cEventID );
            delete aevent;
            countEventsDropped++;
        }
        else
        {
            countEventsProcessed++;
        }
    }
}

size_t Audit_Host::getPendingEventsCount()
{
    size_t r;
    mEvents.lock();
    r = auditEvents.size();
    mEvents.unlock();
    return r;
}

uint64_t Audit_Host::getCountEventsDropped() const
{
    return countEventsDropped;
}

uint64_t Audit_Host::getCountEventsProcessed() const
{
    return countEventsProcessed;
}
