#ifndef SERVERAPP_H
#define SERVERAPP_H

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>

using Poco::Util::OptionSet;


class ServerAPP: public Poco::Util::ServerApplication
{
public:
    ServerAPP();
    Poco::Logger * getLogger();
    ~ServerAPP();

protected:
    void reinitialize(Application &self);
    void initialize(Application& self);
    void uninitialize();
    void defineOptions(OptionSet& options);

    void handleHelp(const std::string& , const std::string&);
    void handleConfig(const std::string &, const std::string &value);
    void handleRules(const std::string &, const std::string &value);
    void handleActions(const std::string &, const std::string &value);

    int main(const std::vector<std::string>& );

    void displayHelp();
private:
    bool _helpRequested;
};

#endif // SERVERAPP_H
