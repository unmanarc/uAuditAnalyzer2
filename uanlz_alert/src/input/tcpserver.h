#ifndef RECEPTORTCPSERVER_H
#define RECEPTORTCPSERVER_H

#include <mdz_net_sockets/acceptor_multithreaded.h>
#include "tcplineprocessor.h"
#include <atomic>
#include <boost/property_tree/ptree.hpp>
#include <mutex>
#include <set>

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
    Mantids::Network::Sockets::Acceptors::MultiThreaded acceptor;
    boost::property_tree::ptree config;
    std::string listenAddr, description;
    uint16_t listenPort;

    std::mutex mt;
    std::set<TCPLineProcessor *> clientProcessors;
};

}}}

#endif // RECEPTORTCPSERVER_H
