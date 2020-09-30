#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include <json/json.h>
#include <cx2_netp_linerecv/linerecv.h>
#include <thread>
#include <atomic>

namespace UANLZ { namespace JSONALERT { namespace Input {

class TCPLineProcessor : public CX2::Network::Line2Line::LineRecv
{
public:
    TCPLineProcessor(CX2::Memory::Streams::Streamable *sock, void * server);
    virtual ~TCPLineProcessor();

    Json::Value getStats();

    void setRemoteIP(const std::string &value);


protected:
    bool processParsedLine(const std::string & line);
private:
    std::atomic<uint64_t> processedLinesCount, droppedElements;

    void * server;
    std::string remoteIP;
};

}}}

#endif // LINERECEPTOR_H
