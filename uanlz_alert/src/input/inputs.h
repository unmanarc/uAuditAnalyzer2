#ifndef INPUTS_H
#define INPUTS_H

#include <mutex>
#include "tcpserver.h"

namespace UANLZ { namespace JSONALERT { namespace Input {

class Inputs
{
public:
    Inputs();
    static json getStats();
    static json readInputs(bool * readOK = nullptr);
    static bool writeInputs(const json & jConfig);

    static bool addInput(json jConfig);
    static bool remInput(const std::string &inputId);

    static bool loadConfig();
    static void startThreads();

    static void setConfigFilePath(const std::string &value);

    static bool getModified();

private:
    static std::list<TCPServer *> tcpServers;
    static std::mutex mt;
    static std::string sConfigFilePath;
    static bool modified;
};

}}}

#endif // INPUTS_H
