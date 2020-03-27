#include "processorthreads_output.h"
#include <thread>

#include "output_postgresql.h"
#include "output_jsontcp.h"

#include "rules/rules.h"

#include "globals_ext.h"

using namespace std;
TS_Queue<Audit_Event> ProcessorThreads_Output::eventsQueue;

void outputProcessorThread(int threadid)
{
    Output_ProgreSQL * db = (Globals::getConfig_main()->get<bool>("OUTPUT/PostgreSQL.Enabled",false))?  new Output_ProgreSQL(threadid) : nullptr;
    Output_JSONTCP * jsontcp = (Globals::getConfig_main()->get<bool>("OUTPUT/JSONTCP.Enabled",true))?  new Output_JSONTCP(threadid) : nullptr;

    while (1)
    {
        map<string, Audit_Var> contentVars;
        Audit_Event * event = ProcessorThreads_Output::popAuditEvent( Globals::getConfig_main()->get<uint32_t>("Processor.EventsFlushTimeoutInMSecs",10000)  );

        // processing event
        if (event)
        {
            event->process();
            if (db) db->logAuditEvent(event);
            if (jsontcp) jsontcp->logAuditEvent(event);

            // Evaluate Rules...
            Rules::evaluate(event->getJSON());

            delete event;
        }
        else
        {
            SERVERAPP->getLogger()->trace("No events so far for output thread #%d, triggering outputs event flush...", threadid);
            if (db) db->logAuditEvent(event);
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

void ProcessorThreads_Output::startDatabaseThreads(const size_t &threads)
{
    for (size_t i=1;i<=threads;i++)
    {
        std::thread tx(outputProcessorThread,i);
        tx.detach();
    }
}

bool ProcessorThreads_Output::pushAuditEvent(Audit_Event *aevent)
{
    // TODO: once filled, start to drop inmmediatly...
    return eventsQueue.push(aevent);
}

Audit_Event *ProcessorThreads_Output::popAuditEvent(const uint32_t & tmout_msecs)
{
    return eventsQueue.pop(tmout_msecs);
}
