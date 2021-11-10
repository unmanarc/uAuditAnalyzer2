#include "tcplineprocessor.h"
#include "tcpserver.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "globals.h"

using namespace std;
using namespace UANLZ::LOG2JSON::Input;
using namespace UANLZ::LOG2JSON;


/*
Mar 19 03:28:34 asd audispd: node=asd.com type=PATH msg=audit(1552987714.396:292): item=0 name="/bin/cat" inode=232100 dev=fd:00 mode=0100755 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:bin_t:s0 objtype=NORMAL cap_fp=0000000000000000 cap_fi=0000000000000000 cap_fe=0 cap_fver=0
*/

// we don't require to parse syslog...
#define REGEX_SYSLOG_HEADER "^<(?<RSYSLOG_HEADER_X>[^>]*)>.{16}[^:]+: "
#define REGEX_SYSLOGIP_HEADER "^<[^>]*><(?<RSYSLOG_HEADER_IP>[^>]+)>.{16}[^:]+: "
#define REGEX_AUDITD_HEADER "node=(?<AUDITD_HEADER_NODE>[^ ]+) type=(?<AUDITD_HEADER_TYPE>[^ ]+) msg=audit\\((?<AUDITD_HEADER_MSG_UNIXTIME>[^\\.]+).(?<AUDITD_HEADER_MSG_MSEC>[^\\:]+):(?<AUDITD_HEADER_MSG_EVENTID>[^\\)]+)\\):"
#define REGEX_AUDTID_CONTENT "(?<AUDITD_DATA>.*)$"

static boost::regex exSyslogIPAuditd(REGEX_SYSLOGIP_HEADER REGEX_AUDITD_HEADER REGEX_AUDTID_CONTENT);
static boost::regex exSyslogAuditd(REGEX_SYSLOG_HEADER REGEX_AUDITD_HEADER REGEX_AUDTID_CONTENT);
static boost::regex exNoHeaderAuditd(REGEX_AUDITD_HEADER REGEX_AUDTID_CONTENT);

TCPLineProcessor::TCPLineProcessor(CX2::Memory::Streams::Streamable *sock, void *server) : LineRecv(sock)
{
    if      (((TCPServer *)server)->getDecoder() == "AUDITD")           this->decoder = DECODER_AUDITD_NOHEADER;
    else if (((TCPServer *)server)->getDecoder() == "SYSLOG+IP+AUDITD") this->decoder = DECODER_AUDITD_SYSLOGWITHIP;
    else                                                                this->decoder = DECODER_AUDITD_SYSLOG;

    this->server = server;

    processedLinesCount=0;
    invalidLinesCount=0;

    setMaxLineSize(128*1024);
}

TCPLineProcessor::~TCPLineProcessor()
{
}

json TCPLineProcessor::getStats()
{
    json v;
    v["ip"] = remoteIP;
    v["processedLinesCount"] = (Json::UInt64)processedLinesCount;
    v["invalidLinesCount"] = (Json::UInt64)invalidLinesCount;
    return v;
}

bool TCPLineProcessor::processParsedLine(const string &line)
{
    if (line.empty()) return true;

    // Separate...
    boost::cmatch whatSyslogAuditdDecomposed;
    boost::cmatch whatDataDecomposed;

    bool receivingIP = decoder == DECODER_AUDITD_SYSLOGWITHIP;
    boost::regex * currentRegex = &exSyslogAuditd;

    if      ( decoder == DECODER_AUDITD_SYSLOG )        currentRegex = &exSyslogAuditd;
    else if ( decoder == DECODER_AUDITD_NOHEADER )      currentRegex = &exNoHeaderAuditd;
    else if ( decoder == DECODER_AUDITD_SYSLOGWITHIP )  currentRegex = &exSyslogIPAuditd;

    if(boost::regex_match(line.c_str(), whatSyslogAuditdDecomposed, *currentRegex))
    {
        string ip               = string(whatSyslogAuditdDecomposed[1].first, whatSyslogAuditdDecomposed[1].second);
        string hostname         = string(whatSyslogAuditdDecomposed[2].first, whatSyslogAuditdDecomposed[2].second);
        string eventType        = string(whatSyslogAuditdDecomposed[3].first, whatSyslogAuditdDecomposed[3].second);

        time_t auditdEventTime  = stol(  string(whatSyslogAuditdDecomposed[4].first, whatSyslogAuditdDecomposed[4].second));
        uint32_t msecs          = stoul( string(whatSyslogAuditdDecomposed[5].first, whatSyslogAuditdDecomposed[5].second));
        uint64_t eventId        = stoull(string(whatSyslogAuditdDecomposed[6].first, whatSyslogAuditdDecomposed[6].second));

        ((TCPServer *)server)->incProcessedLines();
        processedLinesCount++;
        // up to here.. 360k lps
        AuditdEvents::Events_Manager::insertClassContents( {hostname,
                                             receivingIP?ip:remoteIP},
                                            std::make_tuple(auditdEventTime,msecs,eventId),
                                            eventType,
                                            new string(whatSyslogAuditdDecomposed[7].first, whatSyslogAuditdDecomposed[7].second));
        // up to here.. 150k lps
    }
    else
    {
        ((TCPServer *)server)->incInvalidLines();
        invalidLinesCount++;
        Globals::getAppLog()->log1(__func__,remoteIP,CX2::Application::Logs::LEVEL_WARN,"Incomming invalid line: %s", line.c_str());
    }

    return true;
}

void TCPLineProcessor::setRemoteIP(const std::string &value)
{
    remoteIP = value;
}
