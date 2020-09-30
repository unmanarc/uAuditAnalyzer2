#include "serverapp.h"

#include "status_functions.h"

#include "output/output_jsontcp.h"
#include "output/output_postgresql.h"
#include "output/outputs_distributionthreads.h"
#include "input/tcplineprocessor.h"
#include "vars/namedefs.h"
#include "rules/rules.h"
#include "globals.h"

#include <Poco/Format.h>
#include <Poco/Util/HelpFormatter.h>

#include <cx_net_sockets/socket_tcp.h>
#include <cx_net_sockets/socket_unix.h>
#include <cx_net_threadedacceptor/threadedstreamacceptor.h>

#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Exception;

using namespace std;

static string currentDir = "";

//#define SERVERAPP ((ServerAPP *)Globals::getServerApp())


bool localUnixCommandsServerThread(void *, Socket_Base_Stream * v, const char *)
{
    bool readOK = true;
    unsigned char c = v->readU8(&readOK);
    if (readOK)
    {
        switch(c)
        {
        case 'A':
            // Reload Actions
            v->writeU8(Rules::reloadActions( Globals::getActionsDir() )?'1':'0');
            break;
        case 'R':
            // Reload Rules
            v->writeU8(Rules::reloadRules( Globals::getRulesDir() )?'1':'0');
            break;
        case 'E':
            // Reload Everything
            v->writeU8(Rules::reloadActions( Globals::getActionsDir() ) && Rules::reloadRules( Globals::getRulesDir() )?'1':'0');
            break;
        default:
            break;
        }
    }
    return true;
}


void ServerAPP::handleRules(const string& , const string& value)
{
    Application& app = Application::instance();
    app.logger().information("Loading rules directory: %s", value);
    Globals::setRulesDir(value);
    Rules::reloadRules( value );
}

void ServerAPP::handleActions(const string &, const string &value)
{
    Application& app = Application::instance();
    app.logger().information("Loading actions directory: %s", value);
    Globals::setActionsDir(value);
    Rules::reloadActions( value );
}

