#include "nn/nex/SystemClock.hpp"

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HighResolutionClock.hpp"
#include "nn/nex/TimeProvider.hpp"

namespace nn
{
namespace nex
{
namespace
{

TimeProvider* s_timeProvider = 0;
std::int64_t s_correction = 0;
bool s_initialized = false;
CriticalSection s_criticalSection(1);

}

bool g_systemClockCorrectionEnabled = true;

SystemClock::~SystemClock()
{
}

void SystemClock::ApplyCorrection(Time reference, Time corrected)
{
    s_correction = static_cast<std::int64_t>(corrected - reference);
}

std::uint64_t SystemClock::GetTimeImplCorrectless()
{
    if (!s_initialized)
    {
        std::uint64_t raw;

        s_initialized = true;

        if (s_timeProvider != 0)
            raw = s_timeProvider->GetTime();
        else if (HighResolutionClock::s_defaultToHighResolution)
            raw = HighResolutionClock::GetTime();
        else
            raw = HighResolutionClock::GetLowResolutionTime();

        ApplyCorrection(Time(raw), Time(1));
        return 1;
    }

    if (s_timeProvider != 0)
        return s_timeProvider->GetTime() + s_correction;

    if (HighResolutionClock::s_defaultToHighResolution)
        return HighResolutionClock::GetTime() + s_correction;

    return HighResolutionClock::GetLowResolutionTime() + s_correction;
}

std::uint64_t SystemClock::GetTimeImpl(bool allowRetry)
{
    static Time s_lastTime;

    std::uint64_t raw;

    if (s_timeProvider != 0)
        raw = s_timeProvider->GetTime();
    else if (HighResolutionClock::s_defaultToHighResolution)
        raw = HighResolutionClock::GetTime();
    else
        raw = HighResolutionClock::GetLowResolutionTime();

    if (!s_initialized)
    {
        s_initialized = true;
        s_lastTime = Time(1);
        ApplyCorrection(Time(raw), Time(1));
        return 1;
    }

    if (s_correction >= 0 || raw >= static_cast<std::uint64_t>(-s_correction))
    {
        std::uint64_t now = raw + s_correction;

        if (now >= s_lastTime)
        {
            s_lastTime = Time(now);
            return now;
        }
    }

    if (allowRetry)
    {
        Time corrected(s_lastTime + 1);

        ApplyCorrection(Time(raw), corrected);
        s_lastTime = corrected;
        return corrected;
    }

    return ProtectedGetTime();
}

void SystemClock::RegisterTimeProvider(TimeProvider* provider, bool applyCorrection)
{
    std::uint64_t sample;
    std::uint64_t raw;

    s_criticalSection.Enter();

    if (g_systemClockCorrectionEnabled)
        sample = GetTimeImpl(false);
    else
        sample = GetTimeImplCorrectless();

    g_systemClockCorrectionEnabled = true;
    if (provider != 0)
        g_systemClockCorrectionEnabled = applyCorrection;

    g_lastTime = sample;
    s_timeProvider = provider;

    if (provider != 0)
        raw = provider->GetTime();
    else if (HighResolutionClock::s_defaultToHighResolution)
        raw = HighResolutionClock::GetTime();
    else
        raw = HighResolutionClock::GetLowResolutionTime();

    ApplyCorrection(Time(raw), Time(sample));

    if (g_systemClockCorrectionEnabled)
        g_lastTime = GetTimeImpl(false);
    else
        g_lastTime = GetTimeImplCorrectless();

    s_criticalSection.Leave();
}

std::uint64_t SystemClock::ProtectedGetTime()
{
    std::uint64_t time;

    s_criticalSection.Enter();

    if (g_systemClockCorrectionEnabled)
        time = GetTimeImpl(true);
    else
        time = GetTimeImplCorrectless();

    s_criticalSection.Leave();
    return time;
}

}
}
