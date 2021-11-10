#ifndef VARCONTENT_H
#define VARCONTENT_H

#include <string>
#include <map>
#include <string.h>

namespace UANLZ { namespace LOG2JSON { namespace AuditdEvents {

class Audit_Var
{
public:
    Audit_Var();
    ~Audit_Var();

    // append.
    bool append(Audit_Var & v);
    // parse.
    bool parse(const std::string &auditdEventType, const std::string & varName, const std::string & varValue);

    bool getValid() const;
    bool setValid(bool value);

    char *getRawPure() const;
    std::string getFancy() const;
    size_t getRawPureSize() const;
    uint32_t asUInt32() const;

private:
    void createFancyString();

    bool valid;
    std::string txt;
    std::string fancy;
    size_t rawPureSize;
    char * rawFancy, * rawPure;
};

}}}

#endif // VARCONTENT_H
