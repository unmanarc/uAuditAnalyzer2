#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include <mdz_hlp_functions/json.h>
#include <mdz_proto_linerecv/linerecv.h>
#include <thread>
#include <atomic>

namespace UANLZ { namespace JSONALERT { namespace Input {

class TCPLineProcessor : public Mantids::Network::Line2Line::LineRecv
{
public:
    TCPLineProcessor(Mantids::Memory::Streams::Streamable *sock, void * server);
    virtual ~TCPLineProcessor();

    json getStats();

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
