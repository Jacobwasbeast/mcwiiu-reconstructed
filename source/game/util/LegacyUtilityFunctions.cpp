#include "game/util/LegacyUtilityFunctions.hpp"

extern "C" void AIL_debug_log(const char* format, ...);

void outresfloat()
{
    AIL_debug_log("!-Result = %f");
}

void outresint(uint32_t value)
{
    AIL_debug_log("!-Result = %d", value);
}

void outresint64(uint32_t low, uint32_t high)
{
    AIL_debug_log("!-Result = %llu", high, low, high);
}

void outresuint(uint32_t value)
{
    AIL_debug_log("!-Result = %ld", value);
}

void outresuhex(uint32_t value)
{
    AIL_debug_log("!-Result = 0x%lX", value);
}

int32_t ourtoupper(char value)
{
    if (value >= 'a' && value <= 'z')
    {
        value = static_cast<char>(value - 0x20);
    }
    return static_cast<int32_t>(value);
}

int OnlyNum(const char* value)
{
    const char* current = value;
    while (*current != '\0')
    {
        if ((current != value || *current != '-') &&
            ((*current < '0' || *current > '9') && *current != '.'))
        {
            return 0;
        }
        ++current;
    }
    return 1;
}
