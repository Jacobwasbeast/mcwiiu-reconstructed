#include "nn/nex/RoutingTable.hpp"

#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

bool RoutingTable::RTEntry::IsExpired(unsigned int uiElapsed) const
{
    unsigned int remaining = m_timeRemaining;
    bool bExpired = true;
    if (remaining > uiElapsed)
    {
        m_timeRemaining = remaining - uiElapsed;
        bExpired = false;
    }
    return bExpired;
}

RoutingTable::RoutingTable(unsigned int uiTimeout)
    : m_entries(), m_timeout(uiTimeout), m_lastCleaning(Time::GetTime())
{
}

bool RoutingTable::Add(const InetAddress& rAddress, const InetAddress& rKey, bool bReplace)
{
    bool bAdded = false;
    if (rKey.GetAddress() != 0)
    {
        if (!(rKey == rAddress))
        {
            std::pair<EntryMap::iterator, bool> result =
                m_entries.insert(EntryMap::value_type(rKey, RTEntry(rAddress, m_timeout)));
            bAdded = result.second;
            if (!bAdded && bReplace)
            {
                result.first->second.m_address = rKey;
                result.first->second.m_timeRemaining = m_timeout;
                bAdded = true;
            }
        }
    }
    return bAdded;
}

bool RoutingTable::Remove(const InetAddress& rKey)
{
    std::pair<EntryMap::iterator, EntryMap::iterator> range = m_entries.equal_range(rKey);
    unsigned int count = 0;
    for (EntryMap::iterator it = range.first; it != range.second; ++it)
        ++count;
    m_entries.erase(range.first, range.second);
    return count == 1;
}

bool RoutingTable::Find(const InetAddress& rKey, InetAddress& rAddress) const
{
    bool bFound = false;
    EntryMap::const_iterator found = m_entries.find(rKey);
    if (found != m_entries.end())
    {
        found->second.m_timeRemaining = m_timeout;
        rAddress = found->second.m_address;
        bFound = true;
    }
    return bFound;
}

void RoutingTable::Clean()
{
    m_entries.clear();
}

void RoutingTable::PeriodicHouseCleaning()
{
    unsigned long long now = Time::GetTime();
    unsigned int elapsed = static_cast<unsigned int>(now) - static_cast<unsigned int>(m_lastCleaning);
    if (elapsed >= 1000)
    {
        m_lastCleaning = now;
        EntryMap::iterator it = m_entries.begin();
        while (it != m_entries.end())
        {
            if (it->second.IsExpired(elapsed))
                m_entries.erase(it++);
            else
                ++it;
        }
    }
}

}
}
