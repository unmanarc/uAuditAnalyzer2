#include "outputs.h"
#include "output/output_base.h"

#include "output/output_jsontcp.h"

using namespace UANLZ::LOG2JSON::Output;

std::list<Output_Base *> Outputs::outputBases;
std::mutex Outputs::mt;

Outputs::Outputs()
{

}

json Outputs::getStats()
{
    json v;
    mt.lock();
    int x=0;
    for (Output_Base * ob : outputBases)
    {
        v[x++] = ob->getStats();
    }
    mt.unlock();
    return v;
}

void Outputs::loadConfig(const std::string &file)
{
    // fill output here.
    Output_JSONTCP * out = new Output_JSONTCP();
    out->loadConfig(file);

    mt.lock();
    outputBases.push_back(out);
    mt.unlock();

}

void Outputs::startThreads()
{
    mt.lock();
    for (Output_Base * ob : outputBases)
    {
        ob->startThread();
    }
    mt.unlock();
}

void Outputs::pushToOutputBases(const json &eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId)
{
    mt.lock();
    for (Output_Base * ob : outputBases)
    {
        ob->logAuditEvent(eventJSON,eventId);
    }
    mt.unlock();
}
