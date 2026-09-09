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

std::uint64_t HighResolutionClock::GetTickCount()
{
    return static_cast<std::uint64_t>(OSGetTime());
}

std::uint64_t HighResolutionClock::GetFrequency()
{
    return static_cast<std::uint64_t>(OSGetSystemInfo()->busClockSpeed >> 2);
}

std::uint64_t HighResolutionClock::GetTime()
{
    return GetTickCount() * 1000 / GetFrequency();
}

std::uint64_t HighResolutionClock::GetLowResolutionTime()
{
    return GetTime();
}

std::uint64_t HighResolutionClock::GetHighResolutionTime()
{
    return GetTickCount() * 1000000 / GetFrequency();
}

}
}
