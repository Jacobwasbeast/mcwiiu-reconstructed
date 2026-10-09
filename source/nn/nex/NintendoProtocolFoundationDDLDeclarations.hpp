#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPROTOCOLFOUNDATIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOPROTOCOLFOUNDATIONDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class NintendoProtocolFoundationDDLDeclarations : public DDLDeclarations
{
public:
    NintendoProtocolFoundationDDLDeclarations();

    virtual ~NintendoProtocolFoundationDDLDeclarations()
    {
    }

    virtual void Init()
    {
    }

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<NintendoProtocolFoundationDDLDeclarations> s_declarations;
};

}
}

#endif
