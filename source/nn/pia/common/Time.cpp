#include "nn/pia/common/Time.hpp"

#include <coreinit/time.h>

namespace nn { namespace pia { namespace common {

void Time::SetNow()
{
    m_tick = static_cast<unsigned long long>(OSGetSystemTime());
}

Time Time::Create(unsigned long long tick)
{
    Time time;
    time.m_tick = tick;
    return time;
}

}}}
