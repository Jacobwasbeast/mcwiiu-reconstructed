#include "nn/pia/ModuleStackChecker.hpp"

namespace nn { namespace pia {


namespace
{
unsigned int g_stackTop;
unsigned int g_stack[24];
}

bool ModuleStackChecker::isInvalid(ModuleType moduleType)
{
    return moduleType < 2 || moduleType > 22;
}

unsigned int ModuleStackChecker::getOrder(ModuleType moduleType)
{
    static const unsigned char first[6] = { 3, 4, 2, 5, 0, 0 };
    static const unsigned char second[6] = { 0, 1, 2, 6, 2, 2 };
    if (moduleType >= 2 && moduleType < 8)
        return first[moduleType - 2];
    if (moduleType > 10 && moduleType < 14)
        return 4;
    if (moduleType > 14 && moduleType < 20)
        return second[moduleType - 15];
    if (moduleType == 22)
        return 2;
    return 0xffffffffU;
}

unsigned int ModuleStackChecker::getTop()
{
    return g_stack[g_stackTop];
}

int ModuleStackChecker::Register(ModuleType moduleType)
{
    if (isInvalid(moduleType))
        return 0x10c07;
    if (g_stackTop >= 23)
        return 0x10c08;
    if (g_stackTop != 0 && getOrder(moduleType) <
        getOrder(static_cast<ModuleType>(getTop())))
        return 0x10c08;
    ++g_stackTop;
    g_stack[g_stackTop] = static_cast<unsigned int>(moduleType);
    return 0;
}

int ModuleStackChecker::Unregister(ModuleType moduleType)
{
    if (isInvalid(moduleType))
        return 0x10c07;
    if (g_stackTop == 0 || getTop() != static_cast<unsigned int>(moduleType))
        return 0x10c08;
    --g_stackTop;
    return 0;
}

}}

