#ifndef RECEPTORTCPSERVER_H
#define RECEPTORTCPSERVER_H

#include <mdz_net_sockets/acceptor_multithreaded.h>
#include "tcplineprocessor.h"
#include <atomic>
#include <set>
#include <boost/property_tree/ptree.hpp>
#include <mutex>

namespace UANLZ { namespace LOG2JSON { namespace Input {

class TCPServer
{
public:
    TCPServer();

    bool loadConfig( const std::string & file );
    void startThreaded();

    json getStats();

    void addClient(TCPLineProcessor * clientProcessor);
    void remClient(TCPLineProcessor * clientProcessor);

    std::string getListenAddr() const;
    uint16_t getListenPort() const;

    void incProcessedLines();
    void incInvalidLines();

private:
    Mantids::Network::Sockets::Acceptors::MultiThreaded acceptor;

    boost::property_tree::ptree config;
    std::string listenAddr, description;
    uint16_t listenPort;
    std::atomic<uint64_t> processedLinesCount, invalidLinesCount;

    std::mutex mt;
    std::set<TCPLineProcessor *> clientProcessors;
};

}}}

#endif // RECEPTORTCPSERVER_H
