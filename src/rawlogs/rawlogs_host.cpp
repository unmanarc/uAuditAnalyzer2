#include "rawlogs_host.h"
#include <string.h>
#include <time.h>

#include "globals_ext.h"

using namespace std;

RawLogs_Host::RawLogs_Host(const std::string &dir, const std::string &hostname)
{
    this->dir = dir;
    this->hostname = hostname;

    char * basename = strdup( hostname.c_str() );
    for (char * basename_iter = basename; *basename_iter; basename_iter++)
    {
        if ( !(
                 (basename_iter[0]>='A' && basename_iter[0]<='Z') ||
                 (basename_iter[0]>='a' && basename_iter[0]<='z') ||
                 (basename_iter[0]>='0' && basename_iter[0]<='9') ||
                 (basename_iter[0]=='.') || (basename_iter[0]=='-')
               )
             )
        {
            basename_iter[0]='_';
        }
    }

    // Replace weird chars...
    for ( char * pos = nullptr ; (pos=strstr(basename,".."))!=nullptr;   )
    {
        pos[0]='_';
        pos[1]='_';
    }

#ifndef WIN32
    fileName = dir + "/" + string(basename) + ".log";
#else
    fileName = dir + "\\" + string(basename) + ".log";
#endif
    free(basename);

    fp = fopen( fileName.c_str(), "a+" );
}

RawLogs_Host::~RawLogs_Host()
{
    if (fp) fclose(fp);
}

void RawLogs_Host::log(const std::string &ipfrom, const std::string &rawLogLine)
{
    if (!fp)
    {
        SERVERAPP->getLogger()->error("Failed to write log line from host (%s): %s", ipfrom, rawLogLine);
        return;
    }

    _mutex.lock();
    fprintf(fp, "IP=%s,ARRIVAL_TIME=%ld: %s\n", ipfrom.c_str(), time(nullptr),  rawLogLine.c_str() );
    fflush(fp);
    _mutex.unlock();
}
