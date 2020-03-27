#ifndef SUBEVENT_CONTAINER_H
#define SUBEVENT_CONTAINER_H

#include "audit_var.h"

#include <json/json.h>

#include <map>
#include <set>

class Audit_Class
{
public:
    Audit_Class();
    ~Audit_Class();

    std::string *getRawInput() const;
    void setRawInput(std::string *value);

    bool processToVarContent();

    std::string getClassName() const;
    void setClassName(const std::string &value);

    Json::Value getJSON();

private:

    void parseVar(const std::string &varName, const std::string &varValue);
    bool mergeSpplitedVars();

    std::set<std::string> getSpplitedVarNames();
    std::string * rawInput;
    std::string className;
    std::map<std::string, Audit_Var> contentVars;

    std::string getVarNameByPos(const std::string & varName, const size_t &pos);
};

#endif // SUBEVENT_CONTAINER_H
