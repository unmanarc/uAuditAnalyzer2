#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <pqxx/pqxx>
#include "output_base.h"

class Output_ProgreSQL : public Output_Base
{
public:
    Output_ProgreSQL(const uint32_t &threadId);
    void logAuditEvent(Audit_Event * aevent);

private:
     bool reconnect();
     pqxx::connection * connection;
};

#endif // DBCONNECTION_H
