#include "processorthreads_output.h"

#include <thread>
#include <chrono>

#include "output_postgresql.h"
#include "output_jsontcp.h"
#include "rules/rules.h"
#include "globals_ext.h"

using namespace std;
TS_Queue<Audit_Event> ProcessorThreads_Output::eventsQueue;
uint32_t ProcessorThreads_Output::pushTimeoutInMS = 0;

std::atomic<double> ProcessorThreads_Output::lastRulesEvaluationTime;
std::atomic<uint64_t> ProcessorThreads_Output::eventsProcessed;
std::atomic<uint64_t> ProcessorThreads_Output::eventsProcessedInLast10Second;
std::atomic<uint64_t> ProcessorThreads_Output::eventsProcessedInThisPeriod;
std::atomic<uint64_t> ProcessorThreads_Output::eventsDropped;


void outputProcessorThread(int threadid)
{   
    std::string threadName = "EVENTS_DEQUEUE";
    pthread_setname_np(pthread_self(), threadName.c_str());


    auto start = chrono::high_resolution_clock::now();
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = finish - start;

    for (;;)
    {
        map<string, Audit_Var> contentVars;
        Audit_Event * event = ProcessorThreads_Output::popAuditEvent( Globals::getConfig_main()->get<uint32_t>("Processor.EventsFlushTimeoutInMSecs",10000) );

        // processing event
        if (event)
        {
            event->process();
            Json::Value eventJSON = event->getJSON();
            delete event;

            ProcessorThreads_Output::addEventsProcessed();


            start = chrono::high_resolution_clock::now();
            // Evaluate Rules...
            Rules::evaluate(eventJSON);
            finish = chrono::high_resolution_clock::now();
            elapsed = finish - start;

            ProcessorThreads_Output::setLastRulesEvaluationTime(elapsed.count());

            // push to outputs...
            Globals::pushToOutputBases(eventJSON, event->getEventId());
        }
        else
        {
            SERVERAPP->getLogger()->trace("No events so far for output thread #%d, triggering outputs event flush...", threadid);
            //if (db) db->logAuditEvent(event);
        }
    }
}

void refreshEventsParsedPerSecond()
{
    std::string threadName = "EVENTS_COUNTER";
    pthread_setname_np(pthread_self(), threadName.c_str());

    for (;;)
    {
        ProcessorThreads_Output::lRefreshEventsParsedPer10Second();
        sleep(10);
    }
}

ProcessorThreads_Output::ProcessorThreads_Output()
{
}

void ProcessorThreads_Output::writeStats(const string &outputDir)
{
    ofstream myfile;
    myfile.open (outputDir + "/processor.queue");
    myfile << "# Processor Queue Current Size / Max Items" << endl;
    myfile << eventsQueue.size() << "/" << eventsQueue.getMaxItems() << endl;
    myfile.close();

    myfile.open (outputDir + "/processor.events_parsed");
    myfile << "# Processor Events parsed" << endl;
    myfile << eventsProcessed << endl;
    myfile.close();

    myfile.open (outputDir + "/processor.events_parsed_last_second");
    myfile << "# Processor Events parsed in the last 10 second (events per 10 seconds)" << endl;
    myfile << eventsProcessedInLast10Second << endl;
    myfile.close();

    myfile.open (outputDir + "/processor.events_dropped");
    myfile << "# Processor Events dropped due to full queue" << endl;
    myfile << eventsDropped << endl;
    myfile.close();

    myfile.open (outputDir + "/processor.last_rule_evaluation_time");
    myfile << "# Processor Rule Last Evaluation Time in milliseconds" << endl;
    myfile << lastRulesEvaluationTime << endl;
    myfile.close();
}

void ProcessorThreads_Output::setQueueSize(const size_t &qSize)
{
    eventsQueue.setMaxItems(qSize);
}

void ProcessorThreads_Output::startProcessorThreads(const size_t &threads)
{
    pushTimeoutInMS = Globals::getConfig_main()->get<uint32_t>("Processor.QueuePushTimeoutInMS",0);
    eventsProcessed = 0;
    eventsDropped = 0;
    eventsProcessedInLast10Second=0;
    eventsProcessedInThisPeriod=0;
    lastRulesEvaluationTime = 0;

    std::thread(refreshEventsParsedPerSecond).detach();

    for (size_t i=1;i<=threads;i++)
    {
        std::thread tx(outputProcessorThread,i);
        tx.detach();
    }
}

bool ProcessorThreads_Output::pushAuditEvent(Audit_Event *aevent)
{
    bool r = eventsQueue.push(aevent, pushTimeoutInMS);
    if (!r) eventsDropped++;
    return r;
}

Audit_Event *ProcessorThreads_Output::popAuditEvent(const uint32_t & tmout_msecs)
{
    return eventsQueue.pop(tmout_msecs);
}

void ProcessorThreads_Output::addEventsProcessed()
{
    eventsProcessed++;
    eventsProcessedInThisPeriod++;
}

void ProcessorThreads_Output::lRefreshEventsParsedPer10Second()
{
    eventsProcessedInLast10Second = (uint64_t)eventsProcessedInThisPeriod;
    eventsProcessedInThisPeriod = 0;
}

void ProcessorThreads_Output::setLastRulesEvaluationTime(double _lastRulesEvaluationTime)
{
    lastRulesEvaluationTime = _lastRulesEvaluationTime;
}
