#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP

#include <stdint.h>

#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbe;

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

    /* Explicitly absent from this Wii U image; declaration is intentional. */
    transport::StationLocation* FindReceivedProbeByConnectionId(unsigned int);

private:
    struct ProbeRecord;

    ProbeRecord* RecordFromNode(common::ListNode* node) const;
    const ProbeRecord* RecordFromLocation(const transport::StationLocation* location) const;
    void Retire(ProbeRecord* record);

    common::ListBase m_active;
    common::ListBase m_free;
    int m_nodeOffset;
    ProbeRecord* m_storage;
    unsigned int m_storageCount;
    unsigned char* m_allocation;
};

}}}

#endif
