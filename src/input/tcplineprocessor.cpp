#include "tcplineprocessor.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "rawlogs/rawlogs_manager.h"

#include "globals_ext.h"

/*
Mar 19 03:28:34 asd audispd: node=asd.com type=PATH msg=audit(1552987714.396:292): item=0 name="/bin/cat" inode=232100 dev=fd:00 mode=0100755 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:bin_t:s0 objtype=NORMAL cap_fp=0000000000000000 cap_fi=0000000000000000 cap_fe=0 cap_fver=0
*/

// we don't require to parse syslog...
#define REGEX_SYSLOG "^<[^>]*>.{16}[^:]+: "
#define REGEX_AUDITD_HEADER "node=(?<AUDITD_HEADER_NODE>[^ ]+) type=(?<AUDITD_HEADER_TYPE>[^ ]+) msg=audit\\((?<AUDITD_HEADER_MSG_UNIXTIME>[^\\.]+).(?<AUDITD_HEADER_MSG_MSEC>[^\\:]+):(?<AUDITD_HEADER_MSG_EVENTID>[^\\)]+)\\):"
#define REGEX_AUDTID_DATA "(?<AUDITD_DATA>.*)$"

static boost::regex exSyslogAuditdDecomposer(REGEX_SYSLOG REGEX_AUDITD_HEADER REGEX_AUDTID_DATA);
static boost::regex exPureAuditdDecomposer(REGEX_AUDITD_HEADER REGEX_AUDTID_DATA);

void printLastLineProcessed(void * x)
{
    TCPLineProcessor * tlp = (TCPLineProcessor *)x;
    while (tlp->getActive())
    {
        uint32_t localLinesProcessed = tlp->getLinesProcessed();
        SERVERAPP->getLogger()->debug("%u lines queued per minute",localLinesProcessed);
        tlp->resetLinesProcessed();
        sleep(60);
    }
}

TCPLineProcessor::TCPLineProcessor(StreamableObject *sock, bool usingSyslogHeader) : LineRecv(sock)
{
    this->usingSyslogHeader = usingSyslogHeader;
    active = true;
    resetLinesProcessed();
    setMaxLineSize(128*1024);
    t = thread(printLastLineProcessed, this);

    rawLogsEnabled = (Globals::getConfig_main()->get<bool>("OUTPUT/Raw",true));

}

TCPLineProcessor::~TCPLineProcessor()
{
    active = false;
    t.join();
}

void TCPLineProcessor::resetLinesProcessed()
{
    linesProcessed = 0;
}

bool TCPLineProcessor::processParsedLine(const string &line)
{
    if (line.empty()) return true;
    // here we receive the line...
    linesProcessed++;

    // Separate...
    boost::cmatch whatSyslogAuditdDecomposed;
    boost::cmatch whatDataDecomposed;

    if(regex_match(line.c_str(), whatSyslogAuditdDecomposed, usingSyslogHeader? exSyslogAuditdDecomposer : exPureAuditdDecomposer))
    {
        string hostname = string(whatSyslogAuditdDecomposed[1].first, whatSyslogAuditdDecomposed[1].second);
        string eventType = string(whatSyslogAuditdDecomposed[2].first, whatSyslogAuditdDecomposed[2].second);
        uint64_t eventId = stoull(string(whatSyslogAuditdDecomposed[5].first, whatSyslogAuditdDecomposed[5].second));
        time_t auditdEventTime = stol(string(whatSyslogAuditdDecomposed[3].first, whatSyslogAuditdDecomposed[3].second));
        uint32_t msecs = stoul(string(whatSyslogAuditdDecomposed[4].first, whatSyslogAuditdDecomposed[4].second));

/*        if (rawLogsEnabled && hostname.size())
            RawLogs_Manager::getHostLogger( hostname ).hostLogs->log(remoteIP, line);
        else
            RawLogs_Manager::getHostLogger("invalid2").hostLogs->log(remoteIP, line);*/

        // up to here.. 360k lps
        eventsManager.insertClassContents(hostname,
                                          std::make_tuple(auditdEventTime,msecs,eventId),
                                          eventType,
                                          new string(whatSyslogAuditdDecomposed[6].first, whatSyslogAuditdDecomposed[6].second));
        // up to here.. 150k lps
    }
    else
    {
        SERVERAPP->getLogger()->error("invalid line: %s",line);
/*        if (rawLogsEnabled)
            RawLogs_Manager::getHostLogger("invalid").hostLogs->log(remoteIP, line);*/
    }

    return true;
}

void TCPLineProcessor::setRemoteIP(const std::string &value)
{
    remoteIP = value;
}


uint32_t TCPLineProcessor::getLinesProcessed()
{
    return linesProcessed;
}

bool TCPLineProcessor::getActive()
{
    return active;
}

/*
   #define REGEX_SYSLOG "^(?<SYSLOG_DATE>... .. ..:..:..) (?<SYSLOG_NODE>[^ ]+) (?<SYSLOG_MODULE>[^:]+): "
   cout <<  "------------------" << endl;
   cout << "syslog date header    : " << string(whatSyslogAuditdDecomposed[1].first, whatSyslogAuditdDecomposed[1].second) << endl;
   cout << "syslog node header    : " << string(whatSyslogAuditdDecomposed[2].first, whatSyslogAuditdDecomposed[2].second) << endl;
   cout << "syslog module header  : " << string(whatSyslogAuditdDecomposed[3].first, whatSyslogAuditdDecomposed[3].second) << endl << endl;
*/
