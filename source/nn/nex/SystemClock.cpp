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
int64_t s_correction = 0;
bool s_initialized = false;
CriticalSection s_criticalSection(1);

}

bool g_systemClockCorrectionEnabled = true;

SystemClock::~SystemClock()
{
}

void SystemClock::ApplyCorrection(Time reference, Time corrected)
{
    s_correction = static_cast<int64_t>(corrected - reference);
}

uint64_t SystemClock::GetTimeImplCorrectless()
{
    if (!s_initialized)
    {
        uint64_t raw;

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

uint64_t SystemClock::GetTimeImpl(bool allowRetry)
{
    static Time s_lastTime;

    uint64_t raw;

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

    if (s_correction >= 0 || raw >= static_cast<uint64_t>(-s_correction))
    {
        uint64_t now = raw + s_correction;

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
    uint64_t sample;
    uint64_t raw;

    ScopedCS lock(&s_criticalSection);

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

}

uint64_t SystemClock::ProtectedGetTime()
{
    uint64_t time;

    ScopedCS lock(&s_criticalSection);

    if (g_systemClockCorrectionEnabled)
        time = GetTimeImpl(true);
    else
        time = GetTimeImplCorrectless();

    return time;
}

}
}
