#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include "../events/events_manager.h"

#include <mdz_proto_linerecv/linerecv.h>
#include <thread>
#include <atomic>

namespace UANLZ { namespace LOG2JSON { namespace Input {

enum eDecoders
{
    DECODER_AUDITD_NOHEADER,
    DECODER_AUDITD_SYSLOG,
    DECODER_AUDITD_SYSLOGWITHIP
};

class TCPLineProcessor : public Mantids::Network::Line2Line::LineRecv
{
public:
    TCPLineProcessor(Mantids::Memory::Streams::Streamable *sock, void * server);
    virtual ~TCPLineProcessor();

    json getStats();

//    bool getActive();

//    uint32_t getLinesProcessed();
//    time_t getLastLineProcessed();
//    void resetLinesProcessed();

    void setRemoteIP(const std::string &value);


protected:
    bool processParsedLine(const std::string & line);
private:
    //std::thread t;
    std::atomic<uint64_t> processedLinesCount, invalidLinesCount;

    void * server;
    std::string remoteIP;

    //std::atomic<bool> active;

    eDecoders decoder;
};

}}}

#endif // LINERECEPTOR_H
