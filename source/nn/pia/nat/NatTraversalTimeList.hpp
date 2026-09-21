#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP

#include <cstddef>
#include <type_traits>

#include "nn/pia/ListBase.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"

namespace nn { namespace pia { namespace transport { class StationLocation; } namespace nat {

class NatTraversalTimeList
{
public:
    NatTraversalTimeList();
    virtual ~NatTraversalTimeList();

    NatTraversalTime* Find(const transport::StationLocation& location);
    void Add(const NatTraversalTime& time);
    void Remove(const transport::StationLocation& location);

private:
    struct PoolNode
    {
        common::ListNode node;
        typename std::aligned_storage<sizeof(NatTraversalTime),
                                      alignof(NatTraversalTime)>::type value;
    };

    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    PoolNode* m_storage;
    unsigned int m_storageCount;
    PoolNode m_embeddedNodePool[32];
};

}}}

#endif
