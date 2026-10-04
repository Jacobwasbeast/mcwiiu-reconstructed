#include "nn/nex/ComponentState.hpp"

#include <cwchar>

namespace nn
{
namespace nex
{

const wchar_t* ComponentState::GetType() const
{
    return L"ComponentState";
}

bool ComponentState::IsAKindOf(const wchar_t* pType) const
{
    bool bKind = true;
    if (std::wcscmp(L"ComponentState", pType) != 0)
        bKind = std::wcscmp(L"SystemComponent", pType) == 0;
    return bKind;
}

void ComponentState::EnforceDeclareSysComponentMacro()
{
}

}
}
