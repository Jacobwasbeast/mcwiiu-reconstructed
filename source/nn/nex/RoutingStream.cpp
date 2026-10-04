#include "nn/nex/RoutingStream.hpp"

namespace nn
{
namespace nex
{

namespace
{

unsigned int s_uiHistoryCleaningPeriod = 0;
unsigned int s_uiHistoryLifetime = 10000;

}

RoutingStream::~RoutingStream()
{
}

void RoutingStream::DoWork()
{
    if (s_uiHistoryCleaningPeriod != 0)
    {
        Time now(Time::GetTime());
        if (static_cast<long long>(now - m_lastCleaning) >
            static_cast<long long>(s_uiHistoryCleaningPeriod))
        {
            m_lastCleaning = now;
            CleanHostRoutingHistory();
        }
    }
}

void RoutingStream::CleanHostRoutingHistory()
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    Time now(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_history.erase(it++);
        else
            ++it;
    }

    for (DenyMap::iterator it = m_denied.begin(); it != m_denied.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_denied.erase(it++);
        else
            ++it;
    }

    rCriticalSection.Leave();
}

unsigned int RoutingStream::GetHistoryCount(const InetAddress* pAddress)
{
    if (pAddress == 0)
        return m_history.size();

    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    unsigned int uiCount = 0;
    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end(); ++it)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            ++uiCount;
    }
    rCriticalSection.Leave();
    return uiCount;
}

void RoutingStream::SetDenyForAddress(const InetAddress* pAddress)
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    m_denied[*pAddress] = Time(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            m_history.erase(it++);
        else
            ++it;
    }
    rCriticalSection.Leave();
}

}
}
