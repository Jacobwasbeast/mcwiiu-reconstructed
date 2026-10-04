#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP

#include <stdint.h>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

#include "nn/pia/nat/Api.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/nat/NatProbe.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbeList
{
public:
    NatProbeList();
    virtual ~NatProbeList();

    void RemoveNotReceivedProbes(unsigned int connectionId);
    void RemoveDifferentConnectionIdAndSameAddressPortProbes(const transport::StationLocation& location);
    static bool IsSameAddressPortAndConnectionId(const transport::StationLocation&, const transport::StationLocation&);
    NatProbe* FindByAddressPortAndConnectionId(const transport::StationLocation&);
    static bool IsDifferentPortSameAddressAndConnectionId(const transport::StationLocation&, const transport::StationLocation&);
    NatProbe* FindByAddressAndConnectionId(const transport::StationLocation&);
    void RemoveDifferentPortAndSameAddressAndConnectionIdProbes(const transport::StationLocation&);
    NatProbe* FindByConnectionId(unsigned int connectionId);
    bool AddProbe(const NatProbe&);
    bool RemoveSameConnectionIdOtherProbes(const transport::StationLocation&);
    bool ProcessProbe(const transport::StationLocation&, unsigned char);
    void ProcessProbeReply(const transport::StationLocation&, common::Time, unsigned char);
    bool RemoveSameAddressAndConnectionIdProbes(const transport::StationLocation&);
    bool RemoveSameAddressPortAndConnectionIdProbes(const transport::StationLocation&);
    bool RemoveProbesByConnectionId(unsigned int connectionId);
    virtual void Trace(unsigned long long) const;

    common::ListNode* Begin()
    {
        return m_active.Sentinel()->next;
    }

    common::ListNode* End()
    {
        return m_active.Sentinel();
    }

    static NatProbe* ProbeOf(common::ListNode* node)
    {
        return reinterpret_cast<ProbeRecord*>(node)->Probe();
    }

    void Release(common::ListNode* node)
    {
        ProbeOf(node)->~NatProbe();
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
    struct ProbeRecord
    {
        common::ListNode node;
        boost::aligned_storage<sizeof(NatProbe), boost::alignment_of<NatProbe>::value>::type probeStorage;

        NatProbe* Probe()
        {
            return reinterpret_cast<NatProbe*>(&probeStorage);
        }
    };

    static ProbeRecord* RecordFromNode(common::ListNode* node)
    {
        return reinterpret_cast<ProbeRecord*>(node);
    }

    void Retire(ProbeRecord* record)
    {
        record->Probe()->Trace(cTraceBrief);
        Release(&record->node);
    }

    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    ProbeRecord* m_storage;
    unsigned int m_storageCount;
    unsigned char* m_allocation;
};

}}}

#endif
