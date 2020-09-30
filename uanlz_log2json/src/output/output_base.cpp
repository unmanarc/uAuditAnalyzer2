#include "output_base.h"

using namespace UANLZ::LOG2JSON::Output;


Output_Base::Output_Base()
{
}

Output_Base::~Output_Base()
{

}

std::string Output_Base::getDescription() const
{
    return description;
}
