#include "nn/nex/DataStoreDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

DataStoreDDLDeclarations::DataStoreDDLDeclarations()
    : DDLDeclarations(true)
{
}

PseudoGlobalVariable<DataStoreDDLDeclarations> DataStoreDDLDeclarations::s_declarations;

void DataStoreDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
