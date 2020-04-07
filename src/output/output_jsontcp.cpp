#include "output_jsontcp.h"
#include "globals_ext.h"
#include <boost/algorithm/string/replace.hpp>
#include <thread>

void tcpOutputProcessorThread( Output_JSONTCP * output )
{
    output->process();
}

Output_JSONTCP::Output_JSONTCP() : Output_Base()
{
    connection = new Socket_TCP;
    queueValues.setMaxItems(Globals::getConfig_main()->get<size_t>("OUTPUT/JSONTCP.MaxQueuedItems",1000000));
    push_tmout_msecs = Globals::getConfig_main()->get<uint32_t>("OUTPUT/JSONTCP.QueuePushTimeoutInMS",0);
}

Output_JSONTCP::~Output_JSONTCP()
{

}

std::string toUnStyledString(const Json::Value& value)
{
    Json::FastWriter writer;
    return writer.write( value );
}

void Output_JSONTCP::logAuditEvent(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId)
{
    Json::Value * value = new Json::Value;
    // make a copy:
    *value = eventJSON;
    // push, if not report and go.
    if (!queueValues.push(value,0))
    {
        SERVERAPP->getLogger()->error("Output_JSONTCP Queue full, Event %u.%u:%u Dropped...", get<0>(eventId),get<1>(eventId),get<2>(eventId) );
        delete value;
    }
}

void Output_JSONTCP::startThread()
{
    std::thread(tcpOutputProcessorThread,this).detach();
}

void Output_JSONTCP::process()
{
    uint16_t transmitionMode = Globals::getConfig_main()->get<uint16_t>("OUTPUT/JSONTCP.TransmitionMode",0);

    // INITIAL CONNECT ATTEMPT:
    while (!reconnect()) {}

    // LOOP:
    for (;;)
    {
        // Take one JSON value from the queue...
        Json::Value * value = queueValues.pop();
        if (value)
        {
            // Try to send it...
            while (!connection->writeString(transmitionMode==0?toUnStyledString(*value): value->toStyledString() + "=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" ).succeed)
            {
                // if not, try to reconnect.
                while (!reconnect()) {}
            }
            delete value;
        }
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
        SERVERAPP->getLogger()->error("JSONTCP connection error: %s" ,msg);
        sleep(Globals::getConfig_main()->get<uint32_t>("OUTPUT/JSONTCP.ReconnectSleepTimeInSecs",3));
        return false;
    }
    else
    {
        SERVERAPP->getLogger()->information("JSONTCP connected to %s:%u", server, (uint32_t)port );
        return true;
    }
}
