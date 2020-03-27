#include "output_jsontcp.h"
#include "globals_ext.h"
#include <boost/algorithm/string/replace.hpp>

Output_JSONTCP::Output_JSONTCP(const uint32_t &threadId) : Output_Base(threadId)
{
    connection = new Socket_TCP;
    while (!reconnect()) {}
}

Output_JSONTCP::~Output_JSONTCP()
{

}

std::string toUnStyledString(const Json::Value& value)
{
   Json::FastWriter writer;
   return writer.write( value );
}

void Output_JSONTCP::logAuditEvent(Audit_Event *aevent)
{
    if (!aevent) return;

    uint16_t transmitionMode = Globals::getConfig_main()->get<uint16_t>("OUTPUT/JSONTCP.TransmitionMode",0);

    while (!connection->writeString(
               transmitionMode==0?toUnStyledString(aevent->getJSON()): aevent->getJSON().toStyledString() + "=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" ).succeed)
    {
        while (!reconnect()) {}
    }
}

bool Output_JSONTCP::reconnect()
{
    if (connection)
    {
        delete connection;
        connection = nullptr;
    }

    connection = new Socket_TCP;

    std::string server = Globals::getConfig_main()->get<std::string>("OUTPUT/JSONTCP.Host","127.0.0.1");
    uint16_t port = Globals::getConfig_main()->get<uint16_t>("OUTPUT/JSONTCP.Port",18200);


    if (!connection->connectTo(server.c_str(),
                               port,
                               Globals::getConfig_main()->get<uint32_t>("OUTPUT/JSONTCP.TimeoutSecs",10)))
    {
        std::string msg = connection->getLastError();
        boost::replace_all(msg,"\n",",");
        SERVERAPP->getLogger()->error("JSONTCP connection error (thread #%u): %s", threadId ,msg);
        sleep(Globals::getConfig_main()->get<uint32_t>("OUTPUT/JSONTCP.ReconnectSleepTimeInSecs",3));
        return false;
    }
    else
    {
        SERVERAPP->getLogger()->information("JSONTCP connected to %s:%u (thread %u)", server, (uint32_t)port, threadId );
        return true;
    }
}
