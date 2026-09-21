#include "nn/pia/nat/NatTraversalTimeList.hpp"

#include <cstring>
#include <new>

namespace nn { namespace pia { namespace nat {

namespace
{
NatTraversalTime* valueFromNode(common::ListNode* node, int nodeOffset)
{
    unsigned char* value = reinterpret_cast<unsigned char*>(node) - nodeOffset;
    return reinterpret_cast<NatTraversalTime*>(value + sizeof(common::ListNode));
}

bool sameLocation(const transport::StationLocation& left,
                  const transport::StationLocation& right)
{
    if (left.m_field1c == right.m_field1c)
        return true;
    const unsigned long long key = right.GetKey();
    const unsigned long long recordKey = left.GetKey();
    return key == recordKey;
}
}

NatTraversalTimeList::NatTraversalTimeList()
    : m_active(), m_free(), m_nodeOffset(0), m_storage(m_embeddedNodePool),
      m_storageCount(32)
{
    for (unsigned int i = 0; i < m_storageCount; ++i)
    {
        m_embeddedNodePool[i].node.prev = 0;
        m_embeddedNodePool[i].node.next = 0;
        m_free.InsertBeforeNode(m_free.Sentinel(), &m_embeddedNodePool[i].node);
    }
}

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
        NatTraversalTime* value = valueFromNode(node, m_nodeOffset);
        if (sameLocation(value->m_location, location))
            return value;
    }
    return 0;
}

void NatTraversalTimeList::Add(const NatTraversalTime& time)
{
    NatTraversalTime* value = Find(time.m_location);
    if (value == 0)
    {
        common::ListNode* node = m_free.PopBackNode();
        if (node == 0)
            return;

        value = valueFromNode(node, m_nodeOffset);
        std::memset(value, 0, sizeof(NatTraversalTime));
        ::new (static_cast<void*>(value)) NatTraversalTime();
        m_active.InsertBeforeNode(m_active.Sentinel(), node);
    }

    time.m_location.Trace(0x800);
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
        NatTraversalTime* value = valueFromNode(node, m_nodeOffset);
        if (sameLocation(value->m_location, location))
        {
            value->m_location.Trace(0x800);
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
