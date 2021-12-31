#ifndef OUTPUT_DISTRIBUTIONTHREADS_H
#define OUTPUT_DISTRIBUTIONTHREADS_H

#include <mdz_thr_safecontainers/queue.h>

#include "../events/audit_event.h"

namespace UANLZ { namespace LOG2JSON { namespace AuditdEvents {
class Events_DistributionThreads
{
public:
    Events_DistributionThreads();

    static json stats();

    static void setQueueSize(const size_t & qSize);
    static void startDistributionThreads(const size_t & threads);

    static bool pushAuditEvent(Audit_Event *aevent);
    static Audit_Event * popAuditEvent(const uint32_t &tmout_msecs);

    static void addEventsProcessed();
    static void lRefreshEventsParsedPer10Second();
    static void setLastRulesEvaluationTime(double lastRulesEvaluationTime);


private:
  //  static std::atomic<double> lastRulesEvaluationTime;
    static std::atomic<uint64_t> eventsProcessed, eventsProcessedInLast10Second, eventsProcessedInThisPeriod, eventsDropped;
    static Mantids::Threads::Safe::Queue<Audit_Event> eventsQueue;
    static uint32_t pushTimeoutInMS;
};

}}}

#endif // OUTPUT_DISTRIBUTIONTHREADS_H

