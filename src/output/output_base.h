#ifndef OUTPUT_BASE_H
#define OUTPUT_BASE_H

#include "events/audit_event.h"

class Output_Base
{
public:
    Output_Base(const uint32_t &threadId);
    virtual ~Output_Base();
    virtual void logAuditEvent(Audit_Event * aevent) = 0;

protected:
    uint32_t threadId;

};

#endif // OUTPUT_BASE_H
