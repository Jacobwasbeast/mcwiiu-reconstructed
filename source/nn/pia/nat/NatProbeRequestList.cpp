#include "nn/pia/nat/NatProbeRequestList.hpp"

#include <new>

namespace nn { namespace pia { namespace nat {

NatProbeRequestList::NatProbeRequestList()
    : m_active(), m_free(), m_nodeOffset(0), m_storage(0),
      m_storageCount(0), m_allocation(0)
{
    m_allocation = common::NewArray<unsigned char>(64 * sizeof(PoolNode));
    if (m_allocation != 0)
    {
        m_storage = reinterpret_cast<PoolNode*>(m_allocation);
        m_storageCount = 64;
        for (unsigned int i = 0; i < m_storageCount; ++i)
        {
            PoolNode* element = ::new (static_cast<void*>(
                m_allocation + i * sizeof(PoolNode))) PoolNode;
            element->node.prev = 0;
            element->node.next = 0;
            m_free.InsertBeforeNode(m_free.Sentinel(), &element->node);
        }
    }

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

NatProbeRequestList::~NatProbeRequestList()
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
                m_free.InsertAfterNode(m_free.Sentinel(),
                    reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node)
                                                        + m_nodeOffset));
                node = next;
            }
            m_active.Init();
        }
        common::DeleteArray(m_allocation);
        m_allocation = 0;
    }
}

}}}
