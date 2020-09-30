#include "namedefs.h"

using namespace UANLZ::LOG2JSON::AuditdEvents;

std::set<std::string> NameDefs::textVars;
std::set<std::string> NameDefs::endEventType;

NameDefs::NameDefs()
{

}
// TODO: https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/security_guide/sec-audit_record_types

void NameDefs::init()
{
    textVars.insert("acct");
    textVars.insert("cmd");
    textVars.insert("comm");
    textVars.insert("cwd");
   // textVars.insert("dev");
    textVars.insert("exe");
    textVars.insert("grp");
    textVars.insert("key");
    textVars.insert("name");
    textVars.insert("path");
    textVars.insert("proctitle");
    textVars.insert("reason");

    // TODO: use EOE if available...
    endEventType.insert("SYSTEM_BOOT");
    endEventType.insert("ADD_USER");
    endEventType.insert("ANOM_ABEND");
    //endEventType.insert("ANOM_PROMISCUOUS"); << multi?
    // AVC->ENDS WITH PROCTITLE.
    // BPRM_FCAPS->ENDS WITH PROCTITLE.
    endEventType.insert("CONFIG_CHANGE");
    endEventType.insert("CRED_ACQ");
    endEventType.insert("CRED_DISP");
    endEventType.insert("CRED_REFR");
    endEventType.insert("CRYPTO_KEY_USER");
    endEventType.insert("CRYPTO_SESSION");
    endEventType.insert("DAEMON_START");
    //CWD ->ENDS WITH PROCTITLE.
    //EXECVE ->ENDS WITH PROCTITLE.
    endEventType.insert("LOGIN");
    //NETFILTER_CFG ->ENDS WITH PROCTITLE.
    //PATH ->ENDS WITH PROCTITLE.
    endEventType.insert("PROCTITLE");
    endEventType.insert("SERVICE_START");
    endEventType.insert("SERVICE_STOP");
    //SOCKADDR->ENDS WITH PROCTITLE
    //SYSCALL->ENDS WITH PROCTITLE
    endEventType.insert("USER_ACCT");
    endEventType.insert("USER_AUTH");
    endEventType.insert("USER_CHAUTHTOK");
    endEventType.insert("USER_CMD");
    endEventType.insert("USER_END");
    endEventType.insert("USER_ERR");
    endEventType.insert("USER_LOGIN");
    endEventType.insert("USER_LOGOUT");
    endEventType.insert("USER_MGMT");
    endEventType.insert("USER_ROLE_CHANGE");
    endEventType.insert("USER_START");
}

bool NameDefs::isTextVar(const std::string &varName)
{
    return textVars.find(varName)!=textVars.end();
}

bool NameDefs::isEndEventType(const std::string &eventType)
{
    return endEventType.find(eventType)!=endEventType.end();
}
