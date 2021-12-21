#ifndef INPUTS_H
#define INPUTS_H

#include <mutex>
#include "../input/tcpserver.h"

namespace UANLZ { namespace LOG2JSON { namespace Input {

class Inputs
{
public:
    Inputs();
    static json getStats();
    static void loadConfig( const std::string & file );
    static void startThreads();

private:
    static std::list<TCPServer *> tcpServers;
    static std::mutex mt;
};

}}}

#endif // INPUTS_H
