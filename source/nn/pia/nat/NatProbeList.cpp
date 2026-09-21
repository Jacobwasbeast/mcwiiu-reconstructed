#include "nn/pia/nat/NatProbeList.hpp"

#include <cstddef>
#include <new>
#include <type_traits>

#include "nn/pia/common/Memory.hpp"
#include "nn/pia/nat/NatProbe.hpp"

namespace nn { namespace pia { namespace nat {

struct NatProbeList::ProbeRecord
{
    common::ListNode node;
    typename std::aligned_storage<sizeof(NatProbe), alignof(NatProbe)>::type probeStorage;

    NatProbe* Probe()
    {
        return reinterpret_cast<NatProbe*>(&probeStorage);
    }

    const NatProbe* Probe() const
    {
        return reinterpret_cast<const NatProbe*>(&probeStorage);
    }
};

NatProbeList::NatProbeList()
    : m_active(), m_free(), m_nodeOffset(0), m_storage(0), m_storageCount(0),
      m_allocation(0)
{
    m_allocation = common::NewArray<unsigned char>(64 * sizeof(ProbeRecord));
    if (m_allocation != 0)
    {
        m_storage = reinterpret_cast<ProbeRecord*>(m_allocation);
        m_storageCount = 64;
        for (unsigned int i = 0; i < m_storageCount; ++i)
        {
            ProbeRecord* record = ::new (static_cast<void*>(
                m_allocation + i * sizeof(ProbeRecord))) ProbeRecord;
            record->node.prev = 0;
            record->node.next = 0;
            m_free.InsertBeforeNode(m_free.Sentinel(), &record->node);
        }
    }

    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        node->prev = 0;
        node->next = 0;
        m_free.InsertAfterNode(m_free.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<unsigned char*>(node) + m_nodeOffset));
        node = next;
    }
    m_active.Init();
}

NatProbeList::~NatProbeList()
{
    if (m_allocation != 0)
    {
        if (m_storage != 0)
        {
            common::ListNode* node = m_active.Sentinel()->next;
            while (node != m_active.Sentinel())
            {
                common::ListNode* next = node->next;
                node->prev = 0;
                node->next = 0;
                m_free.InsertAfterNode(m_free.Sentinel(), node);
                node = next;
            }
            m_active.Init();
        }
        common::DeleteArray(m_allocation);
        m_allocation = 0;
    }
}

NatProbeList::ProbeRecord* NatProbeList::RecordFromNode(common::ListNode* node) const
{
    return reinterpret_cast<ProbeRecord*>(reinterpret_cast<char*>(node) - offsetof(ProbeRecord, node));
}

const NatProbeList::ProbeRecord* NatProbeList::RecordFromLocation(
    const transport::StationLocation* location) const
{
    return reinterpret_cast<const ProbeRecord*>(
        reinterpret_cast<const char*>(location) - offsetof(ProbeRecord, probeStorage));
}

void NatProbeList::Retire(ProbeRecord* record)
{
    NatProbe* probe = record->Probe();
    probe->Trace(0x800);
    probe->~NatProbe();
    m_active.EraseNode(&record->node);
    m_free.InsertAfterNode(m_free.Sentinel(), &record->node);
}

bool NatProbeList::IsSameAddressPortAndConnectionId(
    const transport::StationLocation& a, const transport::StationLocation& b)
{
    const unsigned long long bKey = b.GetKey();
    const unsigned long long aKey = a.GetKey();
    return bKey == aKey &&
           a.m_field1c == b.m_field1c;
}

bool NatProbeList::IsDifferentPortSameAddressAndConnectionId(
    const transport::StationLocation& a, const transport::StationLocation& b)
{
    return a.m_address == b.m_address && a.m_field1c == b.m_field1c;
}

NatProbe* NatProbeList::FindByAddressPortAndConnectionId(
    const transport::StationLocation& location)
{
    for (common::ListNode* node = m_active.Sentinel()->next;
         node != m_active.Sentinel(); node = node->next)
    {
        ProbeRecord* record = RecordFromNode(node);
        if (IsSameAddressPortAndConnectionId(*record->Probe(), location))
            return record->Probe();
    }
    return 0;
}

NatProbe* NatProbeList::FindByAddressAndConnectionId(
    const transport::StationLocation& location)
{
    for (common::ListNode* node = m_active.Sentinel()->next;
         node != m_active.Sentinel(); node = node->next)
    {
        ProbeRecord* record = RecordFromNode(node);
        if (IsDifferentPortSameAddressAndConnectionId(*record->Probe(), location))
            return record->Probe();
    }
    return 0;
}

NatProbe* NatProbeList::FindByConnectionId(unsigned int connectionId)
{
    for (common::ListNode* node = m_active.Sentinel()->next;
         node != m_active.Sentinel(); node = node->next)
    {
        ProbeRecord* record = RecordFromNode(node);
        if (record->Probe()->m_field1c == static_cast<int>(connectionId))
            return record->Probe();
    }
    return 0;
}

void NatProbeList::RemoveNotReceivedProbes(unsigned int connectionId)
{
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        NatProbe* probe = record->Probe();
        if (probe->m_startTick == 0 && probe->m_startTickHigh == 0 &&
            probe->m_field1c == static_cast<int>(connectionId))
            Retire(record);
        node = next;
    }
}

void NatProbeList::RemoveDifferentConnectionIdAndSameAddressPortProbes(
    const transport::StationLocation& location)
{
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        NatProbe* probe = record->Probe();
        if (probe->m_field1c != location.m_field1c)
        {
            const unsigned long long locationKey = location.GetKey();
            const unsigned long long probeKey = probe->GetKey();
            if (locationKey == probeKey)
                Retire(record);
        }
        node = next;
    }
}

void NatProbeList::RemoveDifferentPortAndSameAddressAndConnectionIdProbes(
    const transport::StationLocation& location)
{
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        NatProbe* probe = record->Probe();
        if (IsDifferentPortSameAddressAndConnectionId(*probe, location))
            Retire(record);
        node = next;
    }
}

bool NatProbeList::AddProbe(const NatProbe& probe)
{
    probe.Trace(0x800);
    if (probe.m_startTick == 0 && probe.m_startTickHigh == 0)
    {
        RemoveDifferentConnectionIdAndSameAddressPortProbes(probe);
        if (FindByAddressPortAndConnectionId(probe) != 0)
            return true;

        NatProbe* sameAddress = FindByAddressAndConnectionId(probe);
        if (sameAddress == 0)
        {
            NatProbe* sameConnection = FindByConnectionId(
                static_cast<unsigned int>(probe.m_field1c));
            if (sameConnection != 0 && sameConnection->m_rtt != -1 &&
                sameConnection->IsPrivate() != 0 && probe.IsPrivate() == 0)
                return true;
        }
        else
        {
            RemoveDifferentPortAndSameAddressAndConnectionIdProbes(probe);
        }
    }
    else
    {
        RemoveNotReceivedProbes(static_cast<unsigned int>(probe.m_field1c));
        RemoveDifferentConnectionIdAndSameAddressPortProbes(probe);
    }

    common::ListNode* node = m_free.PopBackNode();
    if (node == 0)
        return false;

    ProbeRecord* record = RecordFromNode(node);
    NatProbe* target = ::new (static_cast<void*>(record->Probe())) NatProbe;
    target->m_startTick = 0;
    target->m_startTickHigh = 0;
    target->m_deadlineTick = 0;
    target->m_deadlineTickHigh = 0;
    target->m_ticksPerTimeout = 0;
    target->m_ticksPerTimeoutHigh = 0;
    if (!probe.m_received)
        m_active.InsertBeforeNode(m_active.Sentinel(), node);
    else
        m_active.InsertAfterNode(m_active.Sentinel(), node);

    static_cast<transport::StationLocation&>(*target) =
        static_cast<const transport::StationLocation&>(probe);
    target->m_startTick = probe.m_startTick;
    target->m_startTickHigh = probe.m_startTickHigh;
    target->m_deadlineTick = probe.m_deadlineTick;
    target->m_deadlineTickHigh = probe.m_deadlineTickHigh;
    target->m_received = probe.m_received;
    target->m_sent = probe.m_sent;
    target->m_replied = probe.m_replied;
    target->m_rtt = probe.m_rtt;
    target->m_timeout = probe.m_timeout;
    target->m_probeCount = probe.m_probeCount;
    target->m_probeLimit = probe.m_probeLimit;
    target->m_firstPort = probe.m_firstPort;
    target->m_lastPort = probe.m_lastPort;
    target->m_ticksPerTimeout = probe.m_ticksPerTimeout;
    target->m_ticksPerTimeoutHigh = probe.m_ticksPerTimeoutHigh;
    return true;
}

bool NatProbeList::RemoveSameConnectionIdOtherProbes(
    const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        NatProbe* probe = record->Probe();
        if (probe->m_field1c == location.m_field1c &&
            !IsSameAddressPortAndConnectionId(*probe, location))
        {
            Retire(record);
            removed = true;
        }
        node = next;
    }
    return removed;
}

bool NatProbeList::ProcessProbe(const transport::StationLocation& location,
                                unsigned char timeout)
{
    common::Time now;
    now.SetNow();
    NatProbe* probe = FindByAddressPortAndConnectionId(location);
    if (probe == 0)
        probe = FindByConnectionId(static_cast<unsigned int>(location.m_field1c));

    if (probe == 0)
    {
        location.Trace(0x800);
        const common::TimeSpan duration(
            common::TimeSpan::GetTicksPerMSec().m_tick * 90000LL);
        const NatProbe newProbe(location, now, duration, timeout, 0x32, 0);
        return AddProbe(newProbe);
    }

    const bool updated = probe->UpdateTargetAddress(location);
    RemoveDifferentConnectionIdAndSameAddressPortProbes(*probe);
    probe->m_startTick = static_cast<unsigned int>(now.m_tick);
    probe->m_startTickHigh = static_cast<unsigned int>(now.m_tick >> 32);
    RemoveNotReceivedProbes(static_cast<unsigned int>(probe->m_field1c));
    RemoveSameConnectionIdOtherProbes(*probe);
    return updated;
}

void NatProbeList::ProcessProbeReply(const transport::StationLocation& location,
                                     common::Time sent, unsigned char timeout)
{
    NatProbe* probe = FindByAddressAndConnectionId(location);
    if (probe == 0)
    {
        location.Trace(0x800);
        return;
    }

    probe->UpdateTargetPort(location);
    RemoveDifferentConnectionIdAndSameAddressPortProbes(*probe);
    common::Time now;
    now.SetNow();
    if (sent.m_tick != 0)
        probe->UpdateRtt(now, sent);
    probe->m_startTick = static_cast<unsigned int>(now.m_tick);
    probe->m_startTickHigh = static_cast<unsigned int>(now.m_tick >> 32);
    probe->m_timeout = timeout;
    RemoveNotReceivedProbes(static_cast<unsigned int>(probe->m_field1c));
    RemoveSameConnectionIdOtherProbes(*probe);
}

bool NatProbeList::RemoveSameAddressAndConnectionIdProbes(
    const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        if (IsDifferentPortSameAddressAndConnectionId(*record->Probe(), location))
        {
            Retire(record);
            removed = true;
        }
        node = next;
    }
    return removed;
}

bool NatProbeList::RemoveSameAddressPortAndConnectionIdProbes(
    const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        ProbeRecord* record = RecordFromNode(node);
        if (IsSameAddressPortAndConnectionId(*record->Probe(), location))
        {
            Retire(record);
            removed = true;
        }
        node = next;
    }
    return removed;
}

bool NatProbeList::RemoveProbesByConnectionId(unsigned int connectionId)
{
    bool removed = false;
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        if (RecordFromNode(node)->Probe()->m_field1c == static_cast<int>(connectionId))
        {
            Retire(RecordFromNode(node));
            removed = true;
        }
        node = next;
    }
    return removed;
}

void NatProbeList::Trace(unsigned long long) const
{
}

}}}
