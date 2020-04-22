#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include "events/eventsmanager.h"

#include <cx_protocols_linerecv/linerecv.h>
#include <thread>
#include <atomic>


class TCPLineProcessor : public LineRecv
{
public:
    TCPLineProcessor(StreamableObject *sock, bool usingSyslogHeader, bool usingSyslogIPHeader);
    virtual ~TCPLineProcessor();
    bool getActive();

    uint32_t getLinesProcessed();
    time_t getLastLineProcessed();
    void resetLinesProcessed();

    void setRemoteIP(const std::string &value);

protected:
    bool processParsedLine(const std::string & line);
private:
    std::thread t;

    std::string remoteIP;
    std::atomic<uint32_t> linesProcessed;
    std::atomic<bool> active;

    bool usingSyslogHeader;
    bool usingSyslogIPHeader;
};

#endif // LINERECEPTOR_H
