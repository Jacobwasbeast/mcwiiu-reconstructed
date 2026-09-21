#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP

#include <cstddef>
#include <type_traits>

#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/nat/NatProbeRequest.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbeRequestList
{
public:
    NatProbeRequestList();
    virtual ~NatProbeRequestList();

    unsigned int GetSize();

private:
    struct PoolNode
    {
        common::ListNode node;
        typename std::aligned_storage<sizeof(NatProbeRequest),
                                      alignof(NatProbeRequest)>::type value;
    };

    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    PoolNode* m_storage;
    unsigned int m_storageCount;
    unsigned char* m_allocation;
};

}}}

#endif
