#include "nn/nex/Time.hpp"

#include "nn/nex/SystemClock.hpp"

namespace nn
{
namespace nex
{

uint64_t g_lastTime;

uint64_t Time::GetTime()
{
    uint64_t time;

    if (g_systemClockCorrectionEnabled)
        time = SystemClock::GetTimeImpl(false);
    else
        time = SystemClock::GetTimeImplCorrectless();

    g_lastTime = time;
    return time;
}

uint64_t Time::ConvertTimeoutToDeadline(uint32_t timeout)
{
    if (timeout == 0xffffffffu)
        return 0;

    return Time(GetTime()) + static_cast<int32_t>(timeout);
}

int32_t Time::ConvertDeadlineToTimeout(Time deadline)
{
    uint64_t now;

    if (deadline.m_value == 0)
        return -1;

    now = GetTime();
    if (now < deadline.m_value)
        return static_cast<int32_t>(deadline.m_value - now);

    return 0;
}

}
}
