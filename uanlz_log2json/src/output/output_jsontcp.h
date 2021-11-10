#ifndef OUTPUT_JSONTCP_H
#define OUTPUT_JSONTCP_H

#include "output_base.h"
#include <cx2_net_sockets/socket_tcp.h>
#include <cx2_thr_safecontainers/queue.h>

#include <cx2_hlp_functions/json.h>
#include <boost/property_tree/ptree.hpp>

namespace UANLZ { namespace LOG2JSON { namespace Output {

class Output_JSONTCP : public Output_Base
{
public:
    Output_JSONTCP();
    ~Output_JSONTCP();

    bool loadConfig(const std::string & file);
    void logAuditEvent(const json & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> & eventId);
    void startThread();
    void process();

    json getStats();

private:
    bool reconnect();

    CX2::Threads::Safe::Queue<json> queueValues;

    std::string server;
    uint16_t port;
    CX2::Network::Sockets::Socket_TCP * connection;
    std::atomic<bool> connected;
    std::atomic<uint64_t> dropped, processed;
    uint32_t push_tmout_msecs;
    boost::property_tree::ptree config;
};

}}}
#endif // OUTPUT_JSONTCP_H

