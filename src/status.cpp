#include "status.h"

#include "globals_ext.h"
#include <unistd.h>
#include <thread>

#include "output/processorthreads_output.h"

std::string Status::outputDir;

void processStats()
{
    for (;;)
    {
        Status::run();
    }
}

Status::Status()
{

}

void Status::startThreaded()
{
    outputDir = Globals::getConfig_main()->get<std::string>("Stats.OutputDir","/var/log/uauditdanalyzer/stats");

    if ( !access(outputDir.c_str(),W_OK) )
    {
        std::thread(processStats).detach();
    }
    else
    {
        SERVERAPP->getLogger()->error("Unable to write stats files to output directory '%s', dir not found or not writeable" , outputDir);
    }
}

void Status::run()
{
    // Check input queue
    ProcessorThreads_Output::writeStats(outputDir);
    Globals::writeStatsOnOutputBases(outputDir);
    sleep(Globals::getConfig_main()->get<uint32_t>("Stats.RefreshTimeInSecs",1));
}
