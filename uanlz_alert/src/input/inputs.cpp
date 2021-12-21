#include "inputs.h"
#include <fstream>
#include <cx2_hlp_functions/random.h>

#include "../globals.h"

using namespace UANLZ::JSONALERT::Input;
using namespace CX2::Application;

std::list<TCPServer *> Inputs::tcpServers;
std::mutex Inputs::mt;
std::string Inputs::sConfigFilePath;
bool Inputs::modified = false;

Inputs::Inputs()
{

}

json Inputs::getStats()
{
    json v;

    int x=0;
    mt.lock();
    for (auto i : tcpServers)
    {
        v[x++]=i->getStats();
    }
    mt.unlock();

    return v;
}

json Inputs::readInputs(bool * readOK)
{
    json root;

    if (access(sConfigFilePath.c_str(),R_OK))
    {
        // Create empty JSON for unexistent file:
        json r;

        Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,"Inputs file '%s' not found, creating an empty file.", sConfigFilePath.c_str());

        std::ofstream config_doc(sConfigFilePath, std::ifstream::binary);
        if (!config_doc.is_open()) return false;
        config_doc << r;
        config_doc.close();
    }

    // Read JSON from file:
    std::ifstream config_doc(sConfigFilePath, std::ifstream::binary);

    if (!config_doc.is_open())
    {
        if ( readOK) *readOK = false;
        return root;
    }

    config_doc >> root;

    if ( readOK) *readOK = true;

    return root;
}

bool Inputs::writeInputs(const json &jConfig)
{
    if (readInputs() == jConfig)
        return true;

    std::ofstream config_doc(sConfigFilePath, std::ifstream::binary);
    if (!config_doc.is_open()) return false;
    config_doc << jConfig;
    config_doc.close();
    modified = true;
    return true;
}

bool Inputs::addInput(json jConfig)
{
    bool readOK;
    json root = readInputs(&readOK), rt;
    if (!readOK)
        return false;

    jConfig["id"] = CX2::Helpers::Random::createRandomString(8).c_str();

    std::string listenAddr, description;
    uint16_t listenPort;

    listenAddr = JSON_ASSTRING(jConfig,"ListenAddr","");
    listenPort = JSON_ASUINT(jConfig,"ListenPort",0);
    description = JSON_ASSTRING(jConfig,"Description","");

    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,"Adding JSON input '%s' to config file: Description: '%s' / @%s:%d", JSON_ASCSTRING(jConfig,"id",""),description.c_str(),listenAddr.c_str(),listenPort );

    rt[0]=jConfig;
    // Read array:
    for ( u_int32_t i=0 ; i< root.size(); i++ )
    {
        rt[i+1] = root[i];
    }

    return writeInputs(rt);
}

bool Inputs::remInput(const std::string &inputId)
{
    bool readOK;
    json root = readInputs(&readOK), rt;
    if (!readOK)
        return false;
    // Read array:
    u_int32_t j=0;
    for ( u_int32_t i=0 ; i< root.size(); i++ )
    {
        if ( JSON_ASSTRING(root[i],"id","") != inputId )
            rt[j++] = root[i];
        else
        {
            std::string listenAddr, description;
            uint16_t listenPort;

            listenAddr = JSON_ASSTRING(root[i],"ListenAddr","");
            listenPort = JSON_ASUINT(root[i],"ListenPort",0);
            description = JSON_ASSTRING(root[i],"Description","");

            Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,"Removing JSON input '%s' from config file: Description: '%s' / @%s:%d", JSON_ASCSTRING(root[i],"id",""),description.c_str(),listenAddr.c_str(),listenPort );

        }
    }

    return writeInputs(rt);
}

bool Inputs::loadConfig()
{
    Globals::getAppLog()->log0(__func__,Logs::LEVEL_INFO,"Loading inputs from file: %s", sConfigFilePath.c_str());

    bool readOK;
    json root = readInputs(&readOK);

    if (!readOK)
    {
        return false;
    }

    // Read array:
    for ( u_int32_t i=0 ; i< root.size(); i++ )
    {
        TCPServer * in = new TCPServer();
        if (!in->loadConfig(root[i]))
        {
            delete in;
        }
        else
        {
            mt.lock();
            tcpServers.push_back(in);
            mt.unlock();
        }
    }
    return true;
}

void Inputs::startThreads()
{
    mt.lock();
    for (auto i : tcpServers)
    {
        i->startThreaded();
    }
    mt.unlock();
}

void Inputs::setConfigFilePath(const std::string &value)
{
    sConfigFilePath = value;
}

bool Inputs::getModified()
{
    return modified;
}
