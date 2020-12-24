#include "audit_class.h"

#include "globals.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <boost/regex.hpp>

#include <boost/algorithm/string.hpp>
using namespace boost::algorithm;

static boost::regex exSyslogAuditdData1("(?<AUDITD_DATA_VARNAME>[^=^ ]+)=(?<AUDITD_DATA_VARVALUE>\"[^\"]+\")");
static boost::regex exSyslogAuditdData2("(?<AUDITD_DATA_VARNAME>[^=^ ]+)=(?<AUDITD_DATA_VARVALUE>[^ ^\"]+)");

using namespace std;
using namespace UANLZ::LOG2JSON::AuditdEvents;

Audit_ClassType::Audit_ClassType()
{
    rawInput = nullptr;
}

Audit_ClassType::~Audit_ClassType()
{
    if (rawInput) delete rawInput;
}

string *Audit_ClassType::getRawInput() const
{
    return rawInput;
}

void Audit_ClassType::setRawInput(string *value)
{
    rawInput = value;
}

unsigned char fromHex16(const char *value);

bool Audit_ClassType::processToVarContent()
{
    boost::match_results<string::const_iterator> whatDataDecomposed;
    boost::match_flag_type flags = boost::match_default;

    for (string::const_iterator start = (*rawInput).begin(), end =  (*rawInput).end();
         boost::regex_search(start, end, whatDataDecomposed, exSyslogAuditdData1, flags);
         start = whatDataDecomposed[0].second)
    {
        parseVar(string(whatDataDecomposed[1].first, whatDataDecomposed[1].second),string(whatDataDecomposed[2].first, whatDataDecomposed[2].second));
    }

    for (string::const_iterator start = (*rawInput).begin(), end =  (*rawInput).end();
         boost::regex_search(start, end, whatDataDecomposed, exSyslogAuditdData2, flags);
         start = whatDataDecomposed[0].second)
    {
        parseVar(string(whatDataDecomposed[1].first, whatDataDecomposed[1].second),string(whatDataDecomposed[2].first, whatDataDecomposed[2].second));
    }

    return mergeSpplitedVars();
}

string Audit_ClassType::getClassTypeName() const
{
    return classTypeName;
}

void Audit_ClassType::setClassTypeName(const string &value)
{
    classTypeName = value;
}

Json::Value Audit_ClassType::getJSON()
{
    Json::Value v;

    bool normal = true;
    if (classTypeName == "EXECVE")
    {
        if (contentVars.find("argc")!=contentVars.end())
        {
            normal = false;
            v["argc"] = contentVars["argc"].getFancy();
            std::string cmdline;
            for (uint32_t i=0; i<contentVars["argc"].asUInt() && i<65536; i++)
            {
                std::string curVar = "a" + to_string(i);
                if (contentVars.find(curVar)!=contentVars.end())
                {
                    v["argv"][i] = contentVars[curVar].getFancy();
                    cmdline += contentVars[curVar].getFancy() + " ";
                }
                else
                {
                    v["argv"][i] = "__UAA_PARSER_NOT_FOUND__";
                    cmdline += "__UAA_PARSER_NOT_FOUND__ ";
                }
            }
            if (!cmdline.empty()) cmdline.pop_back();
            v["cmdline"] = cmdline;
        }
    }

    if (normal)
    {
        for ( auto & i : contentVars)
        {
            v[i.first] = i.second.getFancy();
        }
    }

    return v;
}

void Audit_ClassType::parseVar(const string &varName, const string &varValue)
{

    if (!contentVars[varName].parse( classTypeName, varName,  varValue ))
    {
        Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_ERR, "Unable to parse variable '%s' with value '%s'", varName.c_str(), varValue.c_str());
    }

    if (classTypeName == "SOCKADDR" && varName=="saddr" && varValue.size()<250)
    {
        string family, laddr, path;
        unsigned short port = 0;

        char data[128];
        memset(data,0,128);
        int i=0;
        for (const char * r_iter = varValue.c_str(); *r_iter;  r_iter+=2)
            data[i++] = fromHex16(r_iter);

        sa_family_t * sockFamily = (sa_family_t *)&data;

        switch (*sockFamily)
        {
        case AF_INET:
        {
            family = "AF_INET";
            sockaddr_in * saddr = (sockaddr_in *)&data;
            char ipAddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(saddr->sin_addr), ipAddr, sizeof(ipAddr));
            port = ntohs(saddr->sin_port);
            laddr = ipAddr;
        } break;
        case AF_INET6:
        {
            family = "AF_INET6";
            sockaddr_in6 * saddr = (sockaddr_in6 *)&data;
            char ipAddr[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(saddr->sin6_addr), ipAddr, sizeof(ipAddr));
            port = ntohs(saddr->sin6_port);
            laddr = ipAddr;
        } break;
        case AF_UNIX:
        {
            family = "AF_UNIX";
            sockaddr_un * saddr = (sockaddr_un *)&data;
            laddr="127.0.0.1";
            path = saddr->sun_path;
        } break;
        default:
            break;
        }

        contentVars["family"].parse( classTypeName, "family",  family );
        contentVars["ipaddr"].parse( classTypeName, "ipaddr",  laddr );
        if (path.size()) contentVars["unix_path"].parse( classTypeName, "unix_path",  path );
        contentVars["port"].parse( classTypeName, "port",  to_string(port) );
    }
}

bool Audit_ClassType::mergeSpplitedVars()
{
    set<string> spplitedVars = getSpplitedVarNames();
    for (const auto & varName : spplitedVars)
    {
        // Take the next vars, append to the first one and remove the extra...
        string varNameWithZeroPos = getVarNameByPos(varName,0);
        contentVars[varName].append(contentVars[varNameWithZeroPos]);
        contentVars.erase(varNameWithZeroPos);

        size_t i=1;
        for (std::string varNameWithPos = getVarNameByPos(varName,i++); contentVars.find(varNameWithPos) != contentVars.end();  varNameWithPos = getVarNameByPos(varName,i++))
        {
            // Append each variable, and then destroy it.
            contentVars[varName].append(contentVars[varNameWithPos]);
            contentVars.erase(varNameWithPos);
        }
    }


    // Find any variable ending with ]
    for (const auto & i : contentVars)
    {
        if (ends_with(i.first,"]"))
        {
            Globals::getAppLog()->log0(__func__,CX2::Application::Logs::LEVEL_ERR, "Orphan variable '%s' can't be reconstructed", i.first.c_str());
        }
    }

    return true;
}

set<string> Audit_ClassType::getSpplitedVarNames()
{
    set<string> spplitedVars;
    for ( const auto & i :  contentVars)
    {
        if (ends_with(i.first,"[0]"))
        {
            string varName = i.first;
            varName.pop_back();varName.pop_back();varName.pop_back();
            spplitedVars.insert(varName);
        }
    }
    return spplitedVars;
}

string Audit_ClassType::getVarNameByPos(const string &varName, const size_t & pos)
{
    return varName + "[" + to_string(pos) + "]";
}
