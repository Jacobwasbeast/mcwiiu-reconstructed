#include "nn/pia/common/TimeSpan.hpp"

#include <coreinit/systeminfo.h>

namespace nn { namespace pia { namespace common {

namespace
{
long long GetFrequency()
{
    return static_cast<long long>(OSGetSystemInfo()->busClockSpeed >> 2);
}

TimeSpan s_ticksPerMSec;
bool s_ticksPerMSecInitialized = false;
TimeSpan s_ticksPerSec;
bool s_ticksPerSecInitialized = false;
}

const TimeSpan& TimeSpan::GetTicksPerMSec()
{
    if (!s_ticksPerMSecInitialized)
    {
        s_ticksPerMSecInitialized = true;
        s_ticksPerMSec = TimeSpan(GetFrequency() / 1000);
    }
    return s_ticksPerMSec;
}

const TimeSpan& TimeSpan::GetTicksPerSec()
{
    if (!s_ticksPerSecInitialized)
    {
        s_ticksPerSecInitialized = true;
        s_ticksPerSec = TimeSpan(GetTicksPerMSec().m_tick * 1000);
    }
    return s_ticksPerSec;
}

}}}
