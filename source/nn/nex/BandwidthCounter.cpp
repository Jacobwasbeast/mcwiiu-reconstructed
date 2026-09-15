#include "nn/nex/BandwidthCounter.hpp"

#include <algorithm>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/qChain.hpp"

namespace nn { namespace nex {

namespace
{

typedef BandwidthCounter::CounterList CounterChain;
CounterChain g_bandwidthCounters;
CriticalSection g_bandwidthCounterLock(0x20000000);

}

BandwidthCounter::BandwidthCounter(const String& name)
    : m_pNext(0), m_pPrevious(0), m_name(name), m_minimum(0xffffffff),
      m_maximum(0), m_total(0), m_count(0)
{
    mVTable = 0x104AC7A8u;
    g_bandwidthCounterLock.Enter();
    g_bandwidthCounters.push_back(this);
    g_bandwidthCounterLock.Leave();
}

BandwidthCounter::~BandwidthCounter()
{
    g_bandwidthCounterLock.Enter();
    CounterChain::iterator position = g_bandwidthCounters.begin();
    while (position != g_bandwidthCounters.end() && *position != this)
        ++position;
    if (position != g_bandwidthCounters.end())
        g_bandwidthCounters.erase(position);
    g_bandwidthCounterLock.Leave();
}

void BandwidthCounter::Reset()
{
    m_minimum = 0xffffffff;
    m_maximum = 0;
    m_total = 0;
    m_count = 0;
}

void BandwidthCounter::ResetAll()
{
    g_bandwidthCounterLock.Enter();
    for (CounterChain::iterator it = g_bandwidthCounters.begin();
         it != g_bandwidthCounters.end(); ++it)
        (*it)->Reset();
    g_bandwidthCounterLock.Leave();
}

void BandwidthCounter::Log()
{
}

void BandwidthCounter::LogAll(bool activeOnly)
{
    g_bandwidthCounterLock.Enter();
    for (CounterChain::iterator it = g_bandwidthCounters.begin();
         it != g_bandwidthCounters.end(); ++it)
    {
        if (!activeOnly || (*it)->m_count != 0)
            (*it)->Log();
    }
    g_bandwidthCounterLock.Leave();
}

void BandwidthCounter::operator+=(unsigned int size)
{
    m_total += size;
    ++m_count;
    if (size < m_minimum)
        m_minimum = size;
    if (size > m_maximum)
        m_maximum = size;
}

void BandwidthCounter::Trace(unsigned long long)
{
}

}}
