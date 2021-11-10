#include "output_jsontcp.h"
#include "globals.h"
#include <boost/algorithm/string/replace.hpp>
#include <thread>

using namespace UANLZ::LOG2JSON::Output;
using namespace std;

void tcpOutputProcessorThread( Output_JSONTCP * output )
{
    std::string threadName = "TCP_OUTPUT";
    pthread_setname_np(pthread_self(), threadName.c_str());

    output->process();
}

Output_JSONTCP::Output_JSONTCP() : Output_Base()
{
    connection = new CX2::Network::Sockets::Socket_TCP;
}

Output_JSONTCP::~Output_JSONTCP()
{

}

bool Output_JSONTCP::loadConfig(const std::string &file)
{
    if (!access(file.c_str(),R_OK))
        boost::property_tree::ini_parser::read_ini( file.c_str(),config);
    else
    {
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_ERR, "Unable to load Output JSON TCP Configuration: %s... Invalid Permissions", file.c_str());
        return false;
    }

    server = config.get<std::string>("Host","127.0.0.1");
    port = config.get<uint16_t>("Port",18200);
    description = config.get<std::string>("Description","JSON TCP Output to " + server + ":" + to_string(port));
    queueValues.setMaxItems(config.get<size_t>("MaxQueuedItems",1000000));
    push_tmout_msecs = config.get<uint32_t>("QueuePushTimeoutInMS",0);

    return true;
}

std::string toUnStyledString(const json& value)
{
    Json::FastWriter writer;
    std::string str = writer.write( value );
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

void Output_JSONTCP::logAuditEvent(const json & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId)
{
    json * value = new json;
    // make a copy:
    *value = eventJSON;
    // push, if not report and go.
    if (!queueValues.push(value,push_tmout_msecs))
    {
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_WARN, "Output_JSONTCP Queue full, Event %ld.%d:%lu Dropped...", get<0>(eventId),get<1>(eventId),get<2>(eventId));
        dropped++;
        delete value;
    }
}

void Output_JSONTCP::startThread()
{
    dropped = 0;
    processed = 0;
    std::thread(tcpOutputProcessorThread,this).detach();
}

void Output_JSONTCP::process()
{
    uint16_t transmitionMode = config.get<uint16_t>("TransmitionMode",0);

    // INITIAL CONNECT ATTEMPT:
    while (!reconnect()) {}

    // LOOP:
    for (;;)
    {
        // Take one JSON value from the queue...
        json * value = queueValues.pop();
        if (value)
        {
            // Try to send it...
            while (!connection->writeString(transmitionMode==0?toUnStyledString(*value) + "\n": value->toStyledString()  ).succeed)
            {
                // if not, try to reconnect.
                while (!reconnect()) {}
            }
            processed++;
            delete value;
        }
    }
}

json Output_JSONTCP::getStats()
{
    json v;

    std::string server = config.get<std::string>("Host","127.0.0.1");
    uint16_t port = config.get<uint16_t>("Port",18200);

    v["id"] = server + ":" + to_string(port);
    v["queue"]["size"] = (Json::UInt64)queueValues.size();
    v["queue"]["maxItems"] = (Json::UInt64)queueValues.getMaxItems();
    v["queue"]["dropped"] = (Json::UInt64)dropped;
    v["queue"]["processed"] = (Json::UInt64)processed;
    v["connected"] = (bool)connected;

    return v;
}

bool Output_JSONTCP::reconnect()
{
    if (connection)
    {
        delete connection;
        connection = nullptr;
    }

    connection = new CX2::Network::Sockets::Socket_TCP;


    if (!connection->connectTo(server.c_str(),
                               port,
                               config.get<uint32_t>("TimeoutSecs",10)))
    {
        std::string msg = connection->getLastError();
        boost::replace_all(msg,"\n",",");
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_ERR, "JSONTCP connection error to %s:%d: %s",  server.c_str(), port  ,msg.c_str());
        connected = false;
        sleep(config.get<uint32_t>("ReconnectSleepTimeInSecs",3));
        return false;
    }
    else
    {
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_INFO, "JSONTCP connected to %s:%u",  server.c_str(), port);
        connected = true;
        return true;
    }
}
