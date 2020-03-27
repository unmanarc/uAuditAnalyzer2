#ifndef RAWLOGS_MANAGER_H
#define RAWLOGS_MANAGER_H

/**
    *
    *
    *  DEPRECATED, PLEASE USE RSYSLOG FOR RAW LOGS
    *
    *
*/

#include <string>

#include <boost/thread/pthread/shared_mutex.hpp>

#include <cx_thr_mutex/mutex_rw.h>
#include "rawlogs_host.h"

struct sMutexHandler_RawLogs_Host
{
    sMutexHandler_RawLogs_Host( boost::shared_mutex * mutex  )
    {
        this->mutex = mutex;
        this->mutex->lock_shared();
    }
    ~sMutexHandler_RawLogs_Host( )
    {
        this->mutex->unlock_shared();
    }

    boost::shared_mutex * mutex;
    RawLogs_Host * hostLogs;
};

class RawLogs_Manager
{
public:
    RawLogs_Manager();

    static sMutexHandler_RawLogs_Host getHostLogger( const std::string & hostname );

    static std::string getDir();
    static void setDir(const std::string &value);

private:
    static std::map<std::string,RawLogs_Host *> logsPerHost;
    static boost::shared_mutex _access;
    static std::string dir;
    static Mutex_RW mutex;
};

#endif // RAWLOGS_MANAGER_H
