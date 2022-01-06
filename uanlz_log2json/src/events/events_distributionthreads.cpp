#include "events_distributionthreads.h"

#include <thread>
#include <chrono>

#include "../output/outputs.h"
#include "../globals.h"

using namespace std;
using namespace UANLZ::LOG2JSON::AuditdEvents;
using namespace UANLZ::LOG2JSON;


Mantids::Threads::Safe::Queue<Audit_Event> Events_DistributionThreads::eventsQueue;
uint32_t Events_DistributionThreads::pushTimeoutInMS = 0;

//std::atomic<double> ProcessorThreads_Output::lastRulesEvaluationTime;
std::atomic<uint64_t> Events_DistributionThreads::eventsProcessed;
std::atomic<uint64_t> Events_DistributionThreads::eventsProcessedInLast10Second;
std::atomic<uint64_t> Events_DistributionThreads::eventsProcessedInThisPeriod;
std::atomic<uint64_t> Events_DistributionThreads::eventsDropped;

// TOOD: remover rules de aqui.

void outputDistributionThread(int threadid)
{   
    std::string threadName = "EVENTS_DEQUEUE";
    pthread_setname_np(pthread_self(), threadName.c_str());

//    auto start = chrono::high_resolution_clock::now();
//    auto finish = chrono::high_resolution_clock::now();
//    chrono::duration<double, milli> elapsed = finish - start;

    for (;;)
    {
        map<string, Audit_Var> contentVars;
        Audit_Event * event = Events_DistributionThreads::popAuditEvent( Globals::getConfig_main()->get<uint32_t>("Processor.EventsFlushTimeoutInMSecs",10000) );

        // processing event
        if (event)
        {
            event->process();
            json eventJSON = event->getJSON();
            delete event;

            // push to outputs...
            Output::Outputs::pushToOutputBases(eventJSON, event->getEventId());

            // Account it.
            Events_DistributionThreads::addEventsProcessed();
/*
            start = chrono::high_resolution_clock::now();
            // Evaluate Rules...
            Rules::evaluate(eventJSON);
            finish = chrono::high_resolution_clock::now();
            elapsed = finish - start;

            ProcessorThreads_Output::setLastRulesEvaluationTime(elapsed.count());
*/
        }
        else
        {
            Globals::getAppLog()->log0(__func__,Mantids::Application::Logs::LEVEL_INFO, "No events so far for output thread #%d, triggering no event alert JSON...", threadid);
            json noEventsAlertJSON;
            noEventsAlertJSON["noEventsAlert"] = true;
            Output::Outputs::pushToOutputBases(noEventsAlertJSON, std::make_tuple<time_t, uint32_t, uint64_t>(0,0,0));
        }
    }
}

void refreshEventsParsedPerSecond()
{
    std::string threadName = "EVENTS_COUNTER";
    pthread_setname_np(pthread_self(), threadName.c_str());

    for (;;)
    {
        Events_DistributionThreads::lRefreshEventsParsedPer10Second();
        sleep(10);
    }
}

Events_DistributionThreads::Events_DistributionThreads()
{
}

json Events_DistributionThreads::stats()
{
    json v;
    v["eventsQueue"]["maxItems"] = (Json::UInt64) eventsQueue.getMaxItems();
    v["eventsQueue"]["curSize"] = (Json::UInt64) eventsQueue.size();
    v["eventsQueue"]["dropped"] = (Json::UInt64) eventsDropped;

    v["eventsProcessed"]["last10s"] = (Json::UInt64) eventsProcessedInLast10Second;
    v["eventsProcessed"]["total"] = (Json::UInt64) eventsProcessed;

    return v;
}
/*
    myfile.open (outputDir + "/processor.last_rule_evaluation_time");
    myfile << "# Processor Rule Last Evaluation Time in milliseconds" << endl;
    myfile << lastRulesEvaluationTime << endl;
    myfile.close();
*/

void Events_DistributionThreads::setQueueSize(const size_t &qSize)
{
    eventsQueue.setMaxItems(qSize);
}

void Events_DistributionThreads::startDistributionThreads(const size_t &threads)
{
    pushTimeoutInMS = Globals::getConfig_main()->get<uint32_t>("Processor.QueuePushTimeoutInMS",0);
    eventsProcessed = 0;
    eventsDropped = 0;
    eventsProcessedInLast10Second=0;
    eventsProcessedInThisPeriod=0;
  //  lastRulesEvaluationTime = 0;

    std::thread(refreshEventsParsedPerSecond).detach();

    for (size_t i=1;i<=threads;i++)
    {
        std::thread tx(outputDistributionThread,i);
        tx.detach();
    }
}

bool Events_DistributionThreads::pushAuditEvent(Audit_Event *aevent)
{
    bool r = eventsQueue.push(aevent, pushTimeoutInMS);
    if (!r) eventsDropped++;
    return r;
}

Audit_Event *Events_DistributionThreads::popAuditEvent(const uint32_t & tmout_msecs)
{
    return eventsQueue.pop(tmout_msecs);
}

void Events_DistributionThreads::addEventsProcessed()
{
    eventsProcessed++;
    eventsProcessedInThisPeriod++;
}

void Events_DistributionThreads::lRefreshEventsParsedPer10Second()
{
    eventsProcessedInLast10Second = (uint64_t)eventsProcessedInThisPeriod;
    eventsProcessedInThisPeriod = 0;
}
/*
void ProcessorThreads_Output::setLastRulesEvaluationTime(double _lastRulesEvaluationTime)
{
    lastRulesEvaluationTime = _lastRulesEvaluationTime;
}
*/
