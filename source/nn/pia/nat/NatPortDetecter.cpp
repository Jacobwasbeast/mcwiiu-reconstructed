#include "nn/pia/nat/NatPortDetecter.hpp"

#include "nn/pia/nat/Api.hpp"

namespace nn { namespace pia { namespace nat {

namespace
{
enum
{
    cProbeRepeatCount = 5,
    cMappingTypeDifferent = 2
};
}

NatPortDetecter::NatPortDetecter()
    : NatDetecter(), m_primaryStation(), m_secondaryStation(), m_relay(false), m_startTime()
{
}

NatPortDetecter::~NatPortDetecter()
{
}

Result NatPortDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                                const common::InetAddress& address)
{
    m_startTime.SetNow();
    return NatDetecter::Startup(traverser, context, address);
}

Result NatPortDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                                const common::InetAddress& address,
                                const transport::StationLocation& primary,
                                const transport::StationLocation& secondary, bool relay)
{
    m_primaryStation = primary;
    m_secondaryStation = secondary;
    m_relay = relay;
    Result result = NatDetecter::Startup(traverser, context, address);
    if (result != 0)
        return result;
    StartDetectionJob();
    return 0;
}

void NatPortDetecter::Cleanup()
{
    NatDetecter::Cleanup();
}

void NatPortDetecter::SendNatPortDetectionProbes()
{
    common::InetAddress* secondary = GetPrimaryServerSecondaryPortAddress();
    if (secondary != 0 && m_traverser->m_natProperty.m_mappingType == cMappingTypeDifferent)
        AddNatCheckProbe(NatCheckProbe(*secondary, cPortDetectionProbeRequest), cProbeRepeatCount);
    else
        AddNatCheckProbe(NatCheckProbe(*GetPrimaryServerPrimaryPortAddress(), cPortDetectionProbeRequest),
                         cProbeRepeatCount);
}

void NatPortDetecter::StartSendingNatCheckProbe()
{
    SendDummyProbe();
    SendNatPortDetectionProbes();
}

bool NatPortDetecter::IsReceivedAllMessage()
{
    return GetNatCheckReply(0)->body.portWord != 0;
}

unsigned int NatPortDetecter::HandleResult()
{
    TraceNatCheckReplyArray(cTraceDetail);
    PredictPort(&m_secondaryStation.m_inetAddress);
    if (m_secondaryStation.m_inetAddress.IsValid() == 0)
        return 0;

    m_startTime.SetNow();
    if (m_traverser == 0)
        return 0;

    if (!m_relay)
    {
        m_traverser->SetLocalPublicLocation(m_secondaryStation);
    }
    else
    {
        m_traverser->RelayProbeRequest(m_primaryStation, m_secondaryStation,
                                           m_relay, GetPerceivedPort());
    }
    return 1;
}

unsigned int NatPortDetecter::CheckRetry()
{
    const unsigned int perceivedPort = GetPerceivedPort();
    if (perceivedPort != 0 || m_relay)
    {
        if (NatDetecter::GetCurrentPort() <= perceivedPort || m_relay)
        {
            NatDetecter::SetCurrentPort(static_cast<unsigned short>(perceivedPort));
            return 0;
        }
        if (NatDetecter::RetryBind() != 0)
            return 0;
    }
    return 1;
}

unsigned int NatPortDetecter::GetDetectionTimeout() const
{
    return 2000;
}

}}}
