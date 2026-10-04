#include "nn/nex/StreamBundling.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

StreamBundling::StreamBundling()
    : m_lastFlushTime()
    , m_criticalSection(0x20000000)
{
    m_lastFlushTime = Time();
    m_bEnabled = false;
    m_unknown10 = 0xffff;
    m_unknown12 = 0xffff;
    m_iMaxFlushDelay = 0;
}

StreamBundling::~StreamBundling()
{
}

void StreamBundling::Enable(int iMaxFlushDelay)
{
    ScopedCS lock(&Core::GetInstance()->GetScheduler()->GetCriticalSection());
    m_iMaxFlushDelay = iMaxFlushDelay;
    m_bEnabled = true;
    m_lastFlushTime = Time(Time::GetTime());
}

void StreamBundling::Flush()
{
    ScopedCS lock(&Core::GetInstance()->GetScheduler()->GetCriticalSection());
    m_lastFlushTime = Time(Time::GetTime());
}

void StreamBundling::SetMaxFlushDelay(int iMaxFlushDelay)
{
    m_iMaxFlushDelay = iMaxFlushDelay;
}

void StreamBundling::UpdateFlushTime()
{
    ScopedCS lock(&Core::GetInstance()->GetScheduler()->GetCriticalSection());
    Time currentTime(Time::GetTime());
    if (static_cast<long long>(currentTime - m_lastFlushTime) > m_iMaxFlushDelay)
        m_lastFlushTime = currentTime;
}

Time StreamBundling::GetLastFlushTime()
{
    ScopedCS lock(&Core::GetInstance()->GetScheduler()->GetCriticalSection());
    UpdateFlushTime();
    return m_lastFlushTime;
}

}
}
