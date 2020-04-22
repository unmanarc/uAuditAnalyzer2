#include "audit_var.h"
#include "vars/namedefs.h"
#include <boost/algorithm/string.hpp>
using namespace boost::algorithm;
using namespace std;

Audit_Var::Audit_Var()
{
    valid = false;
    rawPureSize=0;
    rawPure=nullptr;
    rawFancy=nullptr;
}

Audit_Var::~Audit_Var()
{
    if (rawPure) delete [] rawPure;
    if (rawFancy) delete [] rawFancy;
}

bool Audit_Var::append(Audit_Var &v)
{
    if (!v.getValid())
        return setValid(false);

    // Concat raw pures...
    char * n_rawPure = new char[v.getRawPureSize()+rawPureSize+1];
    n_rawPure[v.getRawPureSize()+rawPureSize+1] = 0;
    memcpy(n_rawPure, rawPure, rawPureSize );
    memcpy(n_rawPure+rawPureSize, v.getRawPure(), v.getRawPureSize() );
    delete [] rawPure;
    rawPure = n_rawPure;

    // Concat fancy's
    fancy+=v.getFancy();

    return true;
}

inline bool isHexUpperString(const char *value)
{
    for ( ;value && *value;value++ )
    {
        if (!(*value>='A' && *value<='F') && !(*value>='0' && *value<='9'))
            return false;
    }
    return true;
}

inline bool isNumeric(const char value)
{
    return (value>='0' && value<='9');
}


inline unsigned char fromHex8(const char value)
{
    if (value>='A' && value<='F') return (value-'A')+10;
    else if (value>='0' && value<='9') return (value-'0');
    else if (value>='a' && value<='f') return (value-'a')+10;
    return 0;
}


inline bool isATextVar( const string &auditdEventType, const string &varName )
{
    return (auditdEventType == "EXECVE" && varName.size()>=2 && varName.at(0)=='a' && isNumeric(varName.at(1)) && !ends_with(varName,"n"));
}

unsigned char fromHex16(const char *value)
{
    return fromHex8(value[0])*0x10 + fromHex8(value[1]);
}

void Audit_Var::createFancyString()
{
    rawFancy = new char[rawPureSize+1];
    memcpy(rawFancy,rawPure,rawPureSize);
    rawFancy[rawPureSize] = 0;

    // Replace special bytes..
    for (char * r_iter = rawFancy; r_iter!=(rawFancy+rawPureSize);  r_iter++)
    {
        switch(*r_iter)
        {
        case 0:
        case 33:
            *r_iter = ' ';
            break;
        default:
            break;
        }
    }
    fancy=rawFancy;
}

size_t Audit_Var::getRawPureSize() const
{
    return rawPureSize;
}

uint32_t Audit_Var::asUInt() const
{
    return stoul(fancy);
}

string Audit_Var::getFancy() const
{
    return fancy;
}


char *Audit_Var::getRawPure() const
{
    return rawPure;
}


bool Audit_Var::getValid() const
{
    return valid;
}

bool Audit_Var::setValid(bool value)
{
    valid = value;
    return valid;
}

bool Audit_Var::parse(const string &auditdEventType, const string &varName, const string &varValue)
{
    if (!varValue.size()) return setValid(false);

    if (    (varValue.at(0) == '\'' && varValue.size()>2 && varValue.at(varValue.size()-1)=='\'')
            || (varValue.at(0) == '\"' && varValue.size()>2 && varValue.at(varValue.size()-1)=='\"')
            || (varValue.at(0) == '(' && varValue.size()>2 && varValue.at(varValue.size()-1)==')')
            || (varValue.at(0) == '{' && varValue.size()>2 && varValue.at(varValue.size()-1)=='}'
                )
            )
    {
        rawPureSize = varValue.size()-2;
        rawPure = new char[rawPureSize+1];
        memcpy(rawPure,varValue.c_str()+1,varValue.size()-2);
        rawPure[rawPureSize]=0;

        createFancyString();

        return setValid(true);
    }
    else if ( NameDefs::isTextVar(varName) || isATextVar( auditdEventType, varName ) ) // at this point, if this is text var, then should be HEX (previous if statement check any using "'{ ).
    {
        if (isHexUpperString(varValue.c_str()) && varValue.size()%2==0)
        {
            rawPureSize = (varValue.size()/2);
            rawPure = new char [rawPureSize+1];
            int i=0;
            for (const char * r_iter = varValue.c_str(); *r_iter;  r_iter+=2)
                rawPure[i++] = fromHex16(r_iter);
            rawPure[rawPureSize]=0;

            createFancyString();
            return setValid(true);
        }
        else
        {
            return setValid(false);
        }
    }
    else // Take the value as is.
    {
        rawPureSize = varValue.size();
        rawPure = new char[rawPureSize+1];
        memcpy(rawPure,varValue.c_str(),rawPureSize);
        rawPure[rawPureSize]=0;
        createFancyString();
        return setValid(true);
    }

    return setValid(false);
}
