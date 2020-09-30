#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include "events/events_manager.h"

#include <cx2_netp_linerecv/linerecv.h>
#include <thread>
#include <atomic>

namespace UANLZ { namespace LOG2JSON { namespace Input {

enum eDecoders
{
    DECODER_AUDITD_NOHEADER,
    DECODER_AUDITD_SYSLOG,
    DECODER_AUDITD_SYSLOGWITHIP
};

class TCPLineProcessor : public CX2::Network::Line2Line::LineRecv
{
public:
    TCPLineProcessor(CX2::Memory::Streams::Streamable *sock, void * server);
    virtual ~TCPLineProcessor();

    Json::Value getStats();

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
