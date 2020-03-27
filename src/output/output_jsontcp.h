#ifndef OUTPUT_JSONTCP_H
#define OUTPUT_JSONTCP_H

#include "output_base.h"
#include <cx_net_sockets/socket_tcp.h>

class Output_JSONTCP : public Output_Base
{
public:
    Output_JSONTCP(const uint32_t &threadId);
    ~Output_JSONTCP();
    void logAuditEvent(Audit_Event * aevent);

private:
     Socket_TCP * connection;
     bool reconnect();
};

#endif // OUTPUT_JSONTCP_H
