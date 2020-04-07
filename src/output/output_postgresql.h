#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <pqxx/pqxx>
#include "output_base.h"

class Output_ProgreSQL : public Output_Base
{
public:
    Output_ProgreSQL();
    void logAuditEvent(const Json::Value &eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId);
    void startThread();
    void process();

private:
     bool reconnect();
     pqxx::connection * connection;
};

#endif // DBCONNECTION_H
