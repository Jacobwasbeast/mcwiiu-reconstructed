#include "nn/pia/session/KickoutManageJob.hpp"

namespace nn { namespace pia { namespace session {

KickoutManageJob::KickoutManageJob()
    : common::StepSequenceJob(), m_kickoutReason(0), m_callContext(),
      m_kickoutCallContext(0), m_joinCancelRequested(false), m_joinCancelResult(0)
{
}

KickoutManageJob::~KickoutManageJob()
{
}

void KickoutManageJob::Trace(unsigned long long value) const
{
}

void KickoutManageJob::SetKickoutFlag()
{
}

void KickoutManageJob::KickoutFromNetwork(const common::StationAddress& stationAddress)
{
}

bool KickoutManageJob::Startup()
{
    for (int i = 0; i < cKickoutStationNumMax; ++i)
        m_kickoutStations[i].m_stationIndex = static_cast<StationIndex>(0xfd);
    m_kickoutReason = 0;
    m_joinCancelRequested = false;
    return true;
}

void KickoutManageJob::Cleanup()
{
    for (int i = 0; i < cKickoutStationNumMax; ++i)
        m_kickoutStations[i].m_stationIndex = static_cast<StationIndex>(0xfd);

    if (m_callContext.m_state != common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();

    common::CallContext* kickoutCallContext = m_kickoutCallContext;
    if (kickoutCallContext != 0)
    {
        if (kickoutCallContext->m_state == common::CallContext::cStateInProgress)
            kickoutCallContext->SignalSuccess(0);
        m_kickoutCallContext = 0;
    }
    m_joinCancelRequested = false;
}

bool KickoutManageJob::AssociateKickoutWith(common::CallContext* callContext)
{
    if (m_kickoutCallContext != 0)
        return false;

    m_kickoutReason = 0;
    if (callContext != 0)
    {
        m_kickoutCallContext = callContext;
        callContext->InitiateCall();
    }
    return true;
}

}}}
