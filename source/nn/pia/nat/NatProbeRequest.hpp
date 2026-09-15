#ifndef NN_PIA_NAT_NATPROBEREQUEST_HPP
#define NN_PIA_NAT_NATPROBEREQUEST_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn { namespace pia { namespace nat {

class NatProbeRequest : public transport::StationLocation
{
public:
    NatProbeRequest(const transport::StationLocation& location,
                    const transport::StationConnectionInfo& connectionInfo,
                    bool probeRequested, bool addressValid, common::CallContext* callContext);
    explicit NatProbeRequest(const transport::StationLocation& location);
    virtual ~NatProbeRequest();

    virtual void Trace(unsigned long long value) const;

    transport::StationConnectionInfo m_stationConnectionInfo;
    bool m_probeRequested;
    bool m_addressValid;
    common::Time m_deadline;
    common::CallContext* m_callContext;
};

}}}

#endif
