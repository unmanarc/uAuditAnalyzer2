#ifndef RECEPTORTCPSERVER_H
#define RECEPTORTCPSERVER_H

#include <boost/property_tree/ptree.hpp>
#include <atomic>
#include <mutex>
#include <set>
#include "tcplineprocessor.h"

namespace UANLZ { namespace JSONALERT { namespace Input {

class TCPServer
{
public:
    TCPServer();

    bool loadConfig(const json &jConfig );
    void startThreaded();

    json getStats();

    void addClient(TCPLineProcessor * clientProcessor);
    void remClient(TCPLineProcessor * clientProcessor);

    std::string getListenAddr() const;
    uint16_t getListenPort() const;
private:
    boost::property_tree::ptree config;
    std::string listenAddr, description;
    uint16_t listenPort;

    std::mutex mt;
    std::set<TCPLineProcessor *> clientProcessors;
};

}}}

#endif // RECEPTORTCPSERVER_H
