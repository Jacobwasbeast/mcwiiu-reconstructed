#include "nn/pia/session/SyncClock.hpp"

#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace session {

SyncClock::SyncClock()
    : m_baseTime(0), m_hasBaseTime(false)
{
}

SyncClock::~SyncClock()
{
}

void SyncClock::SetBaseTime(const common::Time& time)
{
    m_baseTime = time.m_tick;
    m_hasBaseTime = true;
}

void SyncClock::ClearBaseTime()
{
    m_baseTime = 0;
    m_hasBaseTime = false;
}

unsigned int SyncClock::GetTime() const
{
    if (!m_hasBaseTime)
    {
        return static_cast<unsigned int>(-1);
    }

    common::Time now;
    now.SetNow();
    return static_cast<unsigned int>((now.m_tick - m_baseTime)
                                     / common::TimeSpan::GetTicksPerMSec().m_tick);
}

}}}
