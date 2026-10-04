#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP

#include "nn/pia/ListBase.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"

namespace nn { namespace pia { namespace transport { class StationLocation; } namespace nat {

class NatTraversalTimeList
{
public:
    NatTraversalTimeList()
        : m_active(), m_free(), m_nodeOffset(0), m_storage(m_embeddedNodePool),
          m_storageCount(cNodeCount)
    {
        for (unsigned int i = 0; i < m_storageCount; ++i)
        {
            m_embeddedNodePool[i].node.prev = 0;
            m_embeddedNodePool[i].node.next = 0;
            m_free.InsertBeforeNode(m_free.Sentinel(), &m_embeddedNodePool[i].node);
        }
    }
    virtual ~NatTraversalTimeList();

    NatTraversalTime* Find(const transport::StationLocation& location);
    void Add(const NatTraversalTime& time);
    void Remove(const transport::StationLocation& location);

    void Clear()
    {
        while (m_active.GetNodeNum() != 0)
        {
            common::ListNode* node = m_active.Sentinel()->prev;
            ValueOf(node)->~NatTraversalTime();
            m_active.EraseNode(node);
            m_free.InsertAfterNode(m_free.Sentinel(),
                reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node) + m_nodeOffset));
        }
    }

private:
    enum { cNodeCount = 32 };

    struct PoolNode
    {
        common::ListNode node;
        unsigned char value[sizeof(NatTraversalTime)];
    };

    static inline NatTraversalTime* ValueOf(common::ListNode* node)
    {
        return reinterpret_cast<NatTraversalTime*>(reinterpret_cast<PoolNode*>(node)->value);
    }

    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    PoolNode* m_storage;
    unsigned int m_storageCount;
    PoolNode m_embeddedNodePool[cNodeCount];
};

}}}

#endif
