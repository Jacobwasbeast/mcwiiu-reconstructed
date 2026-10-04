#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatTraversalTimeList.hpp"

#include <new>

#include "nn/pia/transport/StationLocation.hpp"

namespace nn { namespace pia { namespace nat {

NatTraversalTimeList::~NatTraversalTimeList()
{
    common::ListNode* node = m_active.Sentinel()->next;
    while (node != m_active.Sentinel())
    {
        common::ListNode* next = node->next;
        node->prev = 0;
        node->next = 0;
        m_free.InsertAfterNode(m_free.Sentinel(),
            reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node)
                                                + m_nodeOffset));
        node = next;
    }
    m_active.Init();
}

NatTraversalTime* NatTraversalTimeList::Find(const transport::StationLocation& location)
{
    common::ListNode* sentinel = m_active.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        NatTraversalTime* value = ValueOf(node);
        if (value->m_location.m_field1c == location.m_field1c)
            return value;
        if (value->m_location.m_field1c != location.m_field1c &&
            location.m_stationAddress.GetKey() == value->m_location.m_stationAddress.GetKey())
            return value;
    }
    return 0;
}

void NatTraversalTimeList::Add(const NatTraversalTime& time)
{
    NatTraversalTime* value = Find(time.m_location);
    if (value == 0)
    {
        if (m_free.GetNodeNum() == 0)
            return;
        common::ListNode* node = m_free.PopBackNode();
        if (node != 0)
            node = reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node) - m_nodeOffset);
        value = ValueOf(node);
        if (value != 0)
            ::new (static_cast<void*>(value)) NatTraversalTime();
        m_active.InsertBeforeNode(m_active.Sentinel(), node);
        if (value == 0)
            return;
    }

    time.m_location.Trace(cTraceBrief);
    value->m_location = time.m_location;
    value->m_startTime = time.m_startTime;
    value->m_endTime = time.m_endTime;
    value->m_count = time.m_count;
    value->m_port = time.m_port;
}

void NatTraversalTimeList::Remove(const transport::StationLocation& location)
{
    common::ListNode* sentinel = m_active.Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        common::ListNode* next = node->next;
        NatTraversalTime* value = ValueOf(node);
        if (value->m_location.m_field1c == location.m_field1c ||
            (value->m_location.m_field1c != location.m_field1c &&
             location.m_stationAddress.GetKey() == value->m_location.m_stationAddress.GetKey()))
        {
            value->m_location.Trace(cTraceBrief);
            value->~NatTraversalTime();
            m_active.EraseNode(node);
            m_free.InsertAfterNode(m_free.Sentinel(),
                reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node)
                                                    + m_nodeOffset));
        }
        node = next;
    }
}

}}}
