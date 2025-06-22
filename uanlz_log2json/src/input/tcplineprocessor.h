#ifndef LINERECEPTOR_H
#define LINERECEPTOR_H

#include <mdz_hlp_functions/json.h>
#include <mdz_proto_linerecv/linerecv.h>
#include <atomic>

namespace UANLZ { namespace LOG2JSON { namespace Input {

class TCPLineProcessor : public Mantids::Protocols::Line2Line::LineRecv
{
public:
    TCPLineProcessor(Mantids::Memory::Streams::StreamableObject *sock, void * server);
    virtual ~TCPLineProcessor();

    void setRemoteIP(const std::string &value);

    json getStats();

protected:
    bool processParsedLine(const std::string & line);
private:

    struct SyslogData
    {
        std::string hostName;
        std::string programName;
    };
    struct AuditdHeader
    {
        std::string node;
        std::string type;


        std::tuple<time_t, uint32_t, uint64_t> getEventID()
        {
            return std::make_tuple(auditdEventTime, msecs, eventId);
        }

        time_t auditdEventTime;
        uint32_t msecs;
        uint64_t eventId;

    };
    void addInvalidLine(const std::string & line);

    bool extractIPAddress(const std::string &line, int mode, size_t &readPosition, std::string &ip);
    bool extractSyslogData(const std::string &line, int mode, size_t & readPosition, SyslogData & syslogData);
    bool extractAuditdHeader(const std::string &line, int mode, size_t & readPosition, AuditdHeader & auditdHeader);

    std::atomic<uint64_t> processedLinesCount, invalidLinesCount;


    void * server;
    std::string remoteIP;
};

}}}

#endif // LINERECEPTOR_H
