#include "processorthreads_output.h"
#include <thread>

#include "output_postgresql.h"
#include "output_jsontcp.h"

#include "rules/rules.h"

#include "globals_ext.h"

using namespace std;
TS_Queue<Audit_Event> ProcessorThreads_Output::eventsQueue;
uint32_t ProcessorThreads_Output::pushTimeoutInMS;

void outputProcessorThread(int threadid)
{
    while (1)
    {
        map<string, Audit_Var> contentVars;
        Audit_Event * event = ProcessorThreads_Output::popAuditEvent( Globals::getConfig_main()->get<uint32_t>("Processor.EventsFlushTimeoutInMSecs",10000) );

        // processing event
        if (event)
        {
            event->process();
            Json::Value eventJSON = event->getJSON();
            delete event;

            // Evaluate Rules...
            Rules::evaluate(eventJSON);

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

ProcessorThreads_Output::ProcessorThreads_Output()
{
}

void ProcessorThreads_Output::setQueueSize(const size_t &qSize)
{
    eventsQueue.setMaxItems(qSize);
}

void ProcessorThreads_Output::startProcessorThreads(const size_t &threads)
{
    pushTimeoutInMS = Globals::getConfig_main()->get<uint32_t>("Processor.QueuePushTimeoutInMS",0);

    for (size_t i=1;i<=threads;i++)
    {
        std::thread tx(outputProcessorThread,i);
        tx.detach();
    }
}

bool ProcessorThreads_Output::pushAuditEvent(Audit_Event *aevent)
{
    return eventsQueue.push(aevent, 0);
}

Audit_Event *ProcessorThreads_Output::popAuditEvent(const uint32_t & tmout_msecs)
{
    return eventsQueue.pop(tmout_msecs);
}
