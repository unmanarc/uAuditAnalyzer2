#include "output_postgresql.h"
#include "globals_ext.h"
#include  <boost/algorithm/string/replace.hpp>

Output_ProgreSQL::Output_ProgreSQL() : Output_Base()
{
    connection = nullptr;
    while (!reconnect()) {}
}

void Output_ProgreSQL::logAuditEvent(const Json::Value & eventJSON, const std::tuple<time_t, uint32_t, uint64_t> &eventId)
{
    // TODO: on nullptr flush, other?

    //Globals::getConfig_main()->get<uint32_t>("Database.MaxEventsPerSQLFlush",1000)




/*
Config:
########################################################
# Output data via PostgreSQL
[OUTPUT/PostgreSQL]
# PostgreSQL Output Enabled/Disabled
Enabled=false
# Database connection string (includes the password)
ConnectionString=dbname='uauditanalyzer' user='postgres' password='' hostaddr='127.0.0.1' port='5432'
# Sleep time (in seconds) when a connection failed to attempt a new connection
ReconnectSleepTimeInSecs=3
*/
}

void Output_ProgreSQL::startThread()
{
    // TODO:
}

void Output_ProgreSQL::process()
{
    // TODO:
}

bool Output_ProgreSQL::reconnect()
{
    if (connection)
    {
        delete connection;
        connection = nullptr;
    }
    try
    {
        connection = new pqxx::connection(Globals::getConfig_main()->get<std::string>("OUTPUT/PostgreSQL.ConnectionString","dbname='uauditanalyzer' user='postgres' password='' hostaddr='127.0.0.1' port='5432'"));
        SERVERAPP->getLogger()->information("Database connected.", 0 );

    }
    catch (const std::exception &e)
    {
        std::string msg = e.what();
        boost::replace_all(msg,"\n",",");
        SERVERAPP->getLogger()->error("Database connection error: %s" ,msg);

        sleep(Globals::getConfig_main()->get<uint32_t>("OUTPUT/PostgreSQL.ReconnectSleepTimeInSecs",3));

        return false;
    }
    return true;
}

