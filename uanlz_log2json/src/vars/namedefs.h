#ifndef VARNAMEDEFINITIONS_H
#define VARNAMEDEFINITIONS_H

#include <set>
#include <string>

namespace UANLZ { namespace LOG2JSON { namespace AuditdEvents {

class NameDefs
{
public:
    NameDefs();
    static void init();
    static bool isTextVar( const std::string & varName );
    static bool isEndEventType( const std::string & eventType );

private:
    static std::set<std::string> textVars;
    static std::set<std::string> endEventType;
};

}}}

#endif // VARNAMEDEFINITIONS_H
