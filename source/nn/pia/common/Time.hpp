#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_TIME_HPP

namespace nn { namespace pia { namespace common {

class Time
{
public:
    Time() : m_tick(0) {}

    bool operator<=(const Time& other) const { return m_tick <= other.m_tick; }
    bool operator<(const Time& other) const { return m_tick < other.m_tick; }

    void SetNow();
    static Time Create(unsigned long long tick);
    void Trace(unsigned long long value) const;

    unsigned long long m_tick;
};

}}}

#endif
