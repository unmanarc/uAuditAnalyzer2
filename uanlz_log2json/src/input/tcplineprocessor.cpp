#include "tcplineprocessor.h"
#include "tcpserver.h"

#include "../globals.h"
#include "../events/events_manager.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace UANLZ::LOG2JSON::Input;
using namespace UANLZ::LOG2JSON;

/*
<14>Jun 22 12:19:47 ROCKY2 audisp-syslog[673]: type=CWD msg=audit(1750609187.113:113950): cwd=%22/root%22
<14><IP>Jun 22 12:19:47 ROCKY2 audisp-syslog[673]: type=CWD msg=audit(1750609187.113:113950): cwd=%22/root%22

Mar 19 03:28:34 asd audispd: node=asd.com type=PATH msg=audit(1552987714.396:292): item=0 name="/bin/cat" inode=232100 dev=fd:00 mode=0100755 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:bin_t:s0 objtype=NORMAL cap_fp=0000000000000000 cap_fi=0000000000000000 cap_fe=0 cap_fver=0
*/

TCPLineProcessor::TCPLineProcessor(
    Mantids::Memory::Streams::StreamableObject *sock, void *server)
    : LineRecv(sock)
{
    this->server = server;

    processedLinesCount = 0;
    invalidLinesCount = 0;

    setMaxLineSize(128 * 1024);
}

TCPLineProcessor::~TCPLineProcessor() {}

json TCPLineProcessor::getStats()
{
    json v;
    v["ip"] = remoteIP;
    v["processedLinesCount"] = (Json::UInt64) processedLinesCount;
    v["invalidLinesCount"] = (Json::UInt64) invalidLinesCount;
    return v;
}

#define MODE_RSYSLOG_PURE 0
#define MODE_RSYSLOG_IP 1
#define MODE_AUDITD_PURE 2

bool TCPLineProcessor::processParsedLine(
    const string &line)
{
    if (line.size() < 6)
        return true;

    int mode = MODE_AUDITD_PURE;

    // Dissect here...
    if (line.at(0) == '<')
    {
        mode = MODE_RSYSLOG_PURE;
        // Check if the character after the first '>' is a '<'
        size_t firstBracketPos = line.find('>');
        if (firstBracketPos != string::npos && firstBracketPos + 1 < line.size() && line[firstBracketPos + 1] == '<')
        {
            mode = MODE_RSYSLOG_IP;
        }
    }
    size_t readPosition = 0;

    std::string ip;
    SyslogData syslogData;
    AuditdHeader auditdHeader;

    if (mode == MODE_AUDITD_PURE)
        syslogData.programName = "audispd";

    if (mode == MODE_RSYSLOG_IP || mode == MODE_RSYSLOG_PURE)
    {
        if (!extractIPAddress(line, mode, readPosition, ip))
            return true;

        if (!extractSyslogData(line, mode, readPosition, syslogData))
            return true;
    }

    if (syslogData.programName == "audispd" || syslogData.programName == "audisp-syslog")
    {
        // Now with the rest (line+readPosition), everything inside the first : will be the auditd header.
        // and everything after it, will be the auditd data.

        if (!extractAuditdHeader(line,mode,readPosition,auditdHeader))
        {
            return true;
        }

        // Proccess auditd...
        ((TCPServer *)server)->incProcessedLines();
        processedLinesCount++;
        AuditdEvents::Events_Manager::insertClassContents( { auditdHeader.node.empty()? syslogData.hostName :auditdHeader.node,
                                                            ip.empty()?remoteIP:ip},
                                                            auditdHeader.getEventID(),
                                                            auditdHeader.type,
                                                            new string(line.substr(readPosition))
                                                          );
    }

    return true;
}

void TCPLineProcessor::addInvalidLine(
    const std::string &line)
{
    ((TCPServer *) server)->incInvalidLines();
    invalidLinesCount++;
    LOG_APP->log1(__func__, remoteIP, Mantids::Application::Logs::LEVEL_WARN, "Incoming invalid line: %s", line.c_str());
}

bool TCPLineProcessor::extractIPAddress(
    const std::string &line, int mode, size_t &readPosition, std::string & ip)
{
    // Calculate the IP value and displace the read position
    // The IP came in this format: <PRIVALUE><IP ADDR>
    // PRIVALUE can be anything

    if (mode == MODE_RSYSLOG_PURE)
    {
        // No IP address to extract, just move past the first '>'
        size_t firstBracketPos = line.find('>');
        if (firstBracketPos != string::npos)
        {
            readPosition = firstBracketPos + 1;
        }
        else
        {
            addInvalidLine(line);
            return false;
        }
    }
    else if (mode == MODE_RSYSLOG_IP)
    {
        // Extract IP address between the two '<' and '>'
        size_t firstBracketPos = line.find('>');
        if (firstBracketPos != string::npos)
        {
            size_t secondBracketStart = line.find('<', firstBracketPos + 1);
            if (secondBracketStart != string::npos)
            {
                size_t secondBracketEnd = line.find('>', secondBracketStart + 1);
                if (secondBracketEnd != string::npos && (secondBracketEnd - secondBracketStart) > 1)
                {
                    ip = line.substr(secondBracketStart + 1, secondBracketEnd - secondBracketStart - 1);
                    readPosition = secondBracketEnd + 1;
                }
                else
                {
                    addInvalidLine(line);
                    return false;
                }
            }
            else
            {
                addInvalidLine(line);
                return false;
            }
        }
        else
        {
            addInvalidLine(line);
            return false;
        }
    }

    return true;
}

bool TCPLineProcessor::extractSyslogData(
    const std::string &line, int mode, size_t &readPosition, SyslogData &syslogData)
{
    // Skip the first 16 characters (date part)
    readPosition += 16;

    size_t startPos = readPosition;

    // Find the first space after date to get hostname
    size_t hostEndPos = line.find(' ', startPos);
    if (hostEndPos == string::npos)
    {
        addInvalidLine(line);
        return true; // Invalid format
    }

    syslogData.hostName = line.substr(startPos, hostEndPos - startPos);
    readPosition = hostEndPos + 1;

    // Find the program name until '[' or ' '
    size_t progEndPos = line.find_first_of(" [", readPosition);
    if (progEndPos == string::npos)
    {
        addInvalidLine(line);
        return true; // Invalid format
    }

    syslogData.programName = line.substr(readPosition, progEndPos - readPosition);

    readPosition = progEndPos;

    size_t endSyslogData = line.find(":", readPosition);
    if (progEndPos == string::npos)
    {
        addInvalidLine(line);
        return true; // Invalid format
    }
    readPosition = endSyslogData + 1;


    return true;
}



bool TCPLineProcessor::extractAuditdHeader(
    const std::string &line, int mode, size_t &readPosition, AuditdHeader &auditdHeader)
{
    std::string auditdHeaderStr;
    size_t pos = readPosition;
    bool insideParenthesis = false, found  = false;

    while (pos < line.length())
    {
        if (line[pos] == '(' && !insideParenthesis)
        {
            // We found a parenthesis
            insideParenthesis = true;
        }
        else if (line[pos] == ')' && insideParenthesis)
        {
            insideParenthesis = false;
        }
        else if (line[pos] == ':' && !insideParenthesis)
        {
            // We found the first colon that is not inside parentheses
            auditdHeaderStr = line.substr(readPosition,pos-readPosition);
            readPosition = pos + 1;
            found = true;
            break;
        }

        pos++;
    }

    // If we did not find a valid header, add invalid line and return false.
    if (!found)
    {
        addInvalidLine(line);
        return false;
    }

    std::vector<std::string> tokens;
    boost::split(tokens, auditdHeaderStr, boost::is_any_of(" "), boost::token_compress_on);
    for (const auto& token : tokens) 
    {
        if (token.find("node=") != std::string::npos) 
        {
            auditdHeader.node = token.substr(5);
        } 
        else if (token.find("type=") != std::string::npos) 
        {
            auditdHeader.type = token.substr(5);
        } 
        else if (token.find("msg=audit(") != std::string::npos && token.back() == ')') 
        {
            size_t pos = token.find('(');
            size_t timePos = token.find('.', pos + 1);
            size_t colonPos = token.find(':', timePos + 1);

            if ( pos == std::string::npos ||  timePos ==std::string::npos || colonPos== std::string::npos)
            {
                addInvalidLine(line);
                return false;
            }

            // Verify that all positions are in increasing order
            if (!(pos < timePos && timePos < colonPos))
            {
                addInvalidLine(line);
                return false;
            }

            auditdHeader.auditdEventTime    = strtol(  token.substr(pos + 1     , timePos - pos - 1          ).c_str(), nullptr, 10);
            auditdHeader.msecs              = strtoul( token.substr(timePos + 1 , colonPos - timePos - 1     ).c_str(), nullptr, 10);
            auditdHeader.eventId            = strtoull(token.substr(colonPos + 1, token.size() - colonPos - 2).c_str(), nullptr, 10);
        }
    }

    return true;
}

void TCPLineProcessor::setRemoteIP(
    const std::string &value)
{
    remoteIP = value;
}
