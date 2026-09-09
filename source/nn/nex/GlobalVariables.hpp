#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALVARIABLES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_GLOBALVARIABLES_HPP

#include <cstdint>

namespace nn
{
namespace nex
{

class GlobalVariables
{
public:
    static void AcquireInstance();
    static void ReleaseInstance();
    static std::int32_t GetCount();
    static void ClearCache();
    static void* GetThreadVariableListPtr();
};

}
}

#endif
