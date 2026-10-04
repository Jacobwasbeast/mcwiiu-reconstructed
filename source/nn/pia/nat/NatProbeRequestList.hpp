#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP

#include <cstddef>
#include <new>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/nat/NatProbeRequest.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatProbeRequestList
{
public:
    NatProbeRequestList();
    virtual ~NatProbeRequestList();

    unsigned int GetSize() const
    {
        return m_active.GetNodeNum();
    }

    bool HasFreeNode() const
    {
        return m_free.GetNodeNum() != 0;
    }

    common::ListNode* Begin()
    {
        return m_active.Sentinel()->next;
    }

    common::ListNode* End()
    {
        return m_active.Sentinel();
    }

    static NatProbeRequest* RequestOf(common::ListNode* node)
    {
        return reinterpret_cast<NatProbeRequest*>(&reinterpret_cast<PoolNode*>(node)->value);
    }

    static common::ListNode* NodeOf(NatProbeRequest* request)
    {
        return reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(request) - offsetof(PoolNode, value));
    }

    NatProbeRequest* Allocate()
    {
        common::ListNode* node = m_free.PopBackNode();
        if (node != 0)
            node = reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node) - m_nodeOffset);
        NatProbeRequest* request = RequestOf(node);
        if (request != 0)
            ::new (static_cast<void*>(request)) NatProbeRequest();
        m_active.InsertBeforeNode(m_active.Sentinel(), node);
        return request;
    }

    void Release(common::ListNode* node)
    {
        RequestOf(node)->~NatProbeRequest();
        m_active.EraseNode(node);
        m_free.InsertAfterNode(m_free.Sentinel(),
            reinterpret_cast<common::ListNode*>(reinterpret_cast<unsigned char*>(node) + m_nodeOffset));
    }

    void Clear()
    {
        while (m_active.GetNodeNum() != 0)
            Release(m_active.Sentinel()->prev);
    }

private:
    struct PoolNode
    {
        common::ListNode node;
        boost::aligned_storage<sizeof(NatProbeRequest), boost::alignment_of<NatProbeRequest>::value>::type value;
    };
    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    PoolNode* m_storage;
    unsigned int m_storageCount;
    unsigned char* m_allocation;
};

}
}
}

#endif
