#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <string>
#include <list>
#include <mutex>

#include "output/output_base.h"

namespace UANLZ { namespace LOG2JSON { namespace Output {

class Outputs
{
public:
    Outputs();

    static Json::Value getStats();
    static void loadConfig( const std::string & file );
    static void startThreads();
    static void pushToOutputBases(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId);

private:
    static std::list<Output_Base *> outputBases;
    static std::mutex mt;

};

}}}
#endif // OUTPUTS_H
