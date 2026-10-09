#include "nn/nex/NintendoProtocolFoundationDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

NintendoProtocolFoundationDDLDeclarations::NintendoProtocolFoundationDDLDeclarations()
    : DDLDeclarations(true)
{
}

PseudoGlobalVariable<NintendoProtocolFoundationDDLDeclarations> NintendoProtocolFoundationDDLDeclarations::s_declarations;

void NintendoProtocolFoundationDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
