#include "nn/nex/Time.hpp"

#include "nn/nex/SystemClock.hpp"

namespace nn
{
namespace nex
{

std::uint64_t g_lastTime;

std::uint64_t Time::GetTime()
{
    std::uint64_t time;

    if (g_systemClockCorrectionEnabled)
        time = SystemClock::GetTimeImpl(false);
    else
        time = SystemClock::GetTimeImplCorrectless();

    g_lastTime = time;
    return time;
}

std::uint64_t Time::ConvertTimeoutToDeadline(std::uint32_t timeout)
{
    if (timeout == 0xffffffffu)
        return 0;

    return GetTime() + static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(timeout)));
}

std::int32_t Time::ConvertDeadlineToTimeout(Time deadline)
{
    std::uint64_t now;

    if (deadline.m_value == 0)
        return -1;

    now = GetTime();
    if (now < deadline.m_value)
        return static_cast<std::int32_t>(deadline.m_value - now);

    return 0;
}

}
}
