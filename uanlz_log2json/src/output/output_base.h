#ifndef OUTPUT_BASE_H
#define OUTPUT_BASE_H

#include "events/audit_event.h"
#include <json/json.h>

namespace UANLZ { namespace LOG2JSON { namespace Output {

class Output_Base
{
public:
    Output_Base();
    virtual ~Output_Base();

    virtual bool loadConfig(const std::string & file)=0;
    virtual Json::Value getStats()=0;
    virtual void logAuditEvent(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId) = 0;
    virtual void startThread() = 0;
    virtual void process() = 0;
    std::string getDescription() const;

protected:
    std::string description;

};

}}}
#endif // OUTPUT_BASE_H
