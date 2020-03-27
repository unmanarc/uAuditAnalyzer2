#include "rawlogs_manager.h"


std::string RawLogs_Manager::dir;
boost::shared_mutex RawLogs_Manager::_access;
std::map<std::string,RawLogs_Host *> RawLogs_Manager::logsPerHost;

/*

########################################################
# Output of incomming auditd raw messages
[OUTPUT/Raw]
Enabled=true
Directory=/var/log/uauditd


*/

RawLogs_Manager::RawLogs_Manager()
{
}

sMutexHandler_RawLogs_Host RawLogs_Manager::getHostLogger(const std::string &hostname)
{
    sMutexHandler_RawLogs_Host r(&_access);
    if (logsPerHost.find(hostname) == logsPerHost.end())
    {
        _access.lock_upgrade();
        logsPerHost[hostname] = r.hostLogs = new RawLogs_Host(dir,hostname); // create a new log...
        _access.unlock_upgrade();
    }
    else
    {
        r.hostLogs = logsPerHost[hostname];
    }
    return r;
}

std::string RawLogs_Manager::getDir()
{
    return dir;
}

void RawLogs_Manager::setDir(const std::string &value)
{
    dir = value;
}
