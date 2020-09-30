#ifndef RECEPTORTCPSERVER_H
#define RECEPTORTCPSERVER_H

#include <boost/property_tree/ptree.hpp>
#include <atomic>
#include <mutex>
#include "tcplineprocessor.h"

namespace UANLZ { namespace LOG2JSON { namespace Input {

class TCPServer
{
public:
    TCPServer();

    bool loadConfig( const std::string & file );
    void startThreaded();

    Json::Value getStats();

    void addClient(TCPLineProcessor * clientProcessor);
    void remClient(TCPLineProcessor * clientProcessor);

    std::string getListenAddr() const;
    uint16_t getListenPort() const;
    std::string getDecoder() const;

    void incProcessedLines();
    void incInvalidLines();

private:
    boost::property_tree::ptree config;
    std::string listenAddr, description, decoder;
    uint16_t listenPort;
    std::atomic<uint64_t> processedLinesCount, invalidLinesCount;

    std::mutex mt;
    std::set<TCPLineProcessor *> clientProcessors;
};

}}}

#endif // RECEPTORTCPSERVER_H
