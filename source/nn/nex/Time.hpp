#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIME_HPP

#include <stdint.h>

namespace nn
{
namespace nex
{

class TimeProvider;

extern uint64_t g_lastTime;

class Time
{
public:
    Time() : m_value(0) {}
    explicit Time(uint64_t value) : m_value(value) {}

    operator unsigned long long() const { return m_value; }

    Time Multiply(float factor) const;
    Time Divide(float divisor) const;
    Time Scale(float factor) const;

    static uint64_t GetTime();
    static void Reset();
    static void RegisterTimeProvider(TimeProvider* provider);
    static uint64_t ConvertTimeoutToDeadline(uint32_t timeout);
    static int32_t ConvertDeadlineToTimeout(Time deadline);

private:
    uint64_t m_value;
};

}
}

#endif
