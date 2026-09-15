#include "nn/pia/nat/NatProbeRequest.hpp"

namespace nn { namespace pia { namespace nat {

NatProbeRequest::NatProbeRequest(const transport::StationLocation& location,
                                 const transport::StationConnectionInfo& connectionInfo,
                                 bool probeRequested, bool addressValid,
                                 common::CallContext* callContext)
    : transport::StationLocation(location),
      m_stationConnectionInfo(connectionInfo),
      m_probeRequested(probeRequested),
      m_addressValid(addressValid),
      m_deadline(),
      m_callContext(callContext)
{
}

NatProbeRequest::NatProbeRequest(const transport::StationLocation& location)
    : transport::StationLocation(location),
      m_stationConnectionInfo(),
      m_probeRequested(false),
      m_addressValid(location.m_inetAddress.IsValid() != 0),
      m_deadline(),
      m_callContext(0)
{
}

NatProbeRequest::~NatProbeRequest()
{
}

void NatProbeRequest::Trace(unsigned long long) const
{
}

}}}
