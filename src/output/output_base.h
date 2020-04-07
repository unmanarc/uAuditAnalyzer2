#ifndef OUTPUT_BASE_H
#define OUTPUT_BASE_H

#include "events/audit_event.h"

class Output_Base
{
public:
    Output_Base();
    virtual ~Output_Base();

    virtual void writeStats(const std::string & outputDir)=0;
    virtual void logAuditEvent(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId) = 0;
    virtual void startThread() = 0;
    virtual void process() = 0;

};

#endif // OUTPUT_BASE_H
