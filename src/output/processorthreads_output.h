#ifndef DBTHREADS_H
#define DBTHREADS_H


#include <cx_thr_mutex_queue/ts_queue.h>

#include "events/audit_event.h"

class ProcessorThreads_Output
{
public:
    ProcessorThreads_Output();

    static void writeStats(const std::string &outputDir);

    static void setQueueSize(const size_t & qSize);
    static void startProcessorThreads(const size_t & threads);

    static bool pushAuditEvent(Audit_Event *aevent);
    static Audit_Event * popAuditEvent(const uint32_t &tmout_msecs);

    static void addEventsProcessed();
    static void refreshEventsParsedPerSecond();
    static void setLastRulesEvaluationTime(double lastRulesEvaluationTime);


private:
    static std::atomic<double> lastRulesEvaluationTime;
    static std::atomic<uint64_t> eventsProcessed, eventsProcessedInLastSecond, eventsProcessedInThisSecond, eventsDropped;
    static TS_Queue<Audit_Event> eventsQueue;
    static uint32_t pushTimeoutInMS;
};

#endif // DBTHREADS_H

