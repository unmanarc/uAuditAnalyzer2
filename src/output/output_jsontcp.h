#ifndef OUTPUT_JSONTCP_H
#define OUTPUT_JSONTCP_H

#include "output_base.h"
#include <cx_net_sockets/socket_tcp.h>
#include <cx_thr_mutex_queue/ts_queue.h>
#include <json/json.h>

class Output_JSONTCP : public Output_Base
{
public:
    Output_JSONTCP();
    ~Output_JSONTCP();

    void logAuditEvent(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> & eventId);
    void startThread();
    void process();
    void writeStats(const std::string & outputDir);



private:
    TS_Queue<Json::Value> queueValues;

    Socket_TCP * connection;
    bool reconnect();
    std::atomic<bool> connected;
    uint32_t push_tmout_msecs;
};

#endif // OUTPUT_JSONTCP_H
