#include "nn/nex/HighResolutionClock.hpp"

#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

namespace nn
{
namespace nex
{

bool HighResolutionClock::s_defaultToHighResolution = true;

HighResolutionClock::~HighResolutionClock()
{
}

uint64_t HighResolutionClock::GetTickCount()
{
    return static_cast<uint64_t>(OSGetTime());
}

uint64_t HighResolutionClock::GetFrequency()
{
    return static_cast<uint64_t>(OSGetSystemInfo()->busClockSpeed >> 2);
}

uint64_t HighResolutionClock::GetTime()
{
    return GetTickCount() * 1000 / GetFrequency();
}

uint64_t HighResolutionClock::GetLowResolutionTime()
{
    return GetTime();
}

uint64_t HighResolutionClock::GetHighResolutionTime()
{
    return GetTickCount() * 1000000 / GetFrequency();
}

}
}
