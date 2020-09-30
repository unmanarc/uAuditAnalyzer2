#ifndef SUBEVENT_CONTAINER_H
#define SUBEVENT_CONTAINER_H

#include "audit_var.h"

#include <json/json.h>

#include <map>
#include <set>

namespace UANLZ { namespace LOG2JSON { namespace AuditdEvents {

class Audit_ClassType
{
public:
    Audit_ClassType();
    ~Audit_ClassType();

    std::string *getRawInput() const;
    void setRawInput(std::string *value);

    bool processToVarContent();

    std::string getClassTypeName() const;
    void setClassTypeName(const std::string &value);

    Json::Value getJSON();

private:

    void parseVar(const std::string &varName, const std::string &varValue);
    bool mergeSpplitedVars();

    std::set<std::string> getSpplitedVarNames();
    std::string * rawInput;
    std::string classTypeName;
    std::map<std::string, Audit_Var> contentVars;

    std::string getVarNameByPos(const std::string & varName, const size_t &pos);
};

}}}

#endif // SUBEVENT_CONTAINER_H
