#ifndef RAWLOGS_HOST_H
#define RAWLOGS_HOST_H

/**
    *
    *
    *  DEPRECATED, PLEASE USE RSYSLOG FOR RAW LOGS
    *
    *
*/

#include <string>
#include <mutex>
#include <stdio.h>

class RawLogs_Host
{
public:
    RawLogs_Host(const std::string & dir, const std::string & hostname);
    ~RawLogs_Host();

    void log(const std::string & ipfrom,const std::string & rawLogLine);

private:
    FILE *fp;

    std::mutex _mutex;
    std::string fileName;
    std::string dir;
    std::string hostname;
};

#endif // RAWLOGS_HOST_H
