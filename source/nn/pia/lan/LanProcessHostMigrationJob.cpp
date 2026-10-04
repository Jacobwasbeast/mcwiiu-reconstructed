#include "nn/pia/lan/LanProcessHostMigrationJob.hpp"

#include <algorithm>

namespace nn { namespace pia { namespace lan {

LanProcessHostMigrationJob::LanProcessHostMigrationJob()
    : m_callContext(), m_state(0), m_mode(0), m_host(static_cast<StationIndex>(-1)), m_candidate(static_cast<StationIndex>(-1)), m_rank(0),
      m_candidateCount(0), m_waitCount(0), m_waitLimit(0), m_multiCandidate(false),
      m_hostChanged(false), m_fatalError(false), m_finishDeadline(0), m_reselectDeadline(0)
{
}

LanProcessHostMigrationJob::~LanProcessHostMigrationJob()
{
    CleanupImpl();
}

unsigned int LanProcessHostMigrationJob::LanCleanupOldHostInfo()
{
    m_state = 1;
    m_candidate = static_cast<StationIndex>(-1);
    m_waitCount = 0;
    return m_host < 0 ? 0u : 1u;
}

unsigned int LanProcessHostMigrationJob::LanDecideNextHost()
{
    m_state = 2;
    if (m_candidateCount == 0)
        return 0;
    if (m_candidate < 0)
        m_candidate = static_cast<StationIndex>(0);
    if (m_candidate == m_host)
        m_candidate = static_cast<StationIndex>(m_candidate + 1);
    if (m_candidate >= static_cast<StationIndex>(m_candidateCount))
    {
        m_fatalError = true;
        return 0;
    }
    m_rank = static_cast<unsigned int>(m_candidate);
    m_reselectDeadline = m_waitLimit * 2;
    return 1;
}

unsigned int LanProcessHostMigrationJob::LanPrepareForBecomingHost()
{
    m_state = 5;
    if (m_fatalError)
        return 0;
    m_callContext.Reset();
    m_callContext.InitiateCall();
    m_host = m_candidate;
    return 1;
}

unsigned int LanProcessHostMigrationJob::LanCleanupOldHostInfoOnMultiCandidate()
{
    m_state = 3;
    m_waitCount = 0;
    m_candidate = static_cast<StationIndex>(-1);
    return m_multiCandidate ? 1u : 0u;
}

unsigned int LanProcessHostMigrationJob::LanMakeHostCandidateRanking()
{
    m_state = 4;
    if (!m_multiCandidate || m_candidateCount == 0)
        return 0;
    m_rank = static_cast<unsigned int>(m_candidateCount - 1);
    m_candidate = static_cast<StationIndex>(m_rank);
    return 1;
}

int LanProcessHostMigrationJob::CalcTimeToWaitOtherClients(unsigned int rank, unsigned int count)
{
    if (count == 0 || rank >= count)
        return 0;
    return static_cast<int>((rank * 3000u) + 1000u);
}

unsigned int LanProcessHostMigrationJob::LanSendRankDecision()
{
    m_state = 6;
    m_waitCount = 0;
    m_waitLimit = CalcTimeToWaitOtherClients(m_rank, std::max(1u, m_candidateCount));
    return 1;
}

unsigned int LanProcessHostMigrationJob::LanWaitRankDecision()
{
    if (m_waitCount < m_waitLimit)
    {
        ++m_waitCount;
        return 0;
    }
    return 1;
}

unsigned int LanProcessHostMigrationJob::LanGetMatchMakingClientHost()
{
    m_state = 7;
    return m_candidate >= 0 ? 1u : 0u;
}

unsigned int LanProcessHostMigrationJob::LanWaitMatchMakingClientHost()
{
    if (m_fatalError)
        return 0;
    return m_candidate >= 0 ? 1u : 0u;
}

unsigned int LanProcessHostMigrationJob::LanCheckOldHostDisconnection()
{
    m_state = 8;
    return m_hostChanged ? 0u : 1u;
}

unsigned int LanProcessHostMigrationJob::LanWaitCheckOldHostDisconnection()
{
    return m_hostChanged ? 0u : 1u;
}

void LanProcessHostMigrationJob::StopRankDecisionResending()
{
    m_waitCount = 0;
}

unsigned int LanProcessHostMigrationJob::LanPrepareForBecomingHostMulti()
{
    m_state = 9;
    if (!m_multiCandidate)
        return 0;
    m_callContext.Reset();
    m_callContext.InitiateCall();
    m_host = m_candidate;
    return 1;
}

unsigned int LanProcessHostMigrationJob::LanGetMatchMakingClientHostLastConfirmation()
{
    m_state = 10;
    return m_candidate >= 0 ? 1u : 0u;
}

unsigned int LanProcessHostMigrationJob::LanWaitMatchMakingClientHostLastConfirmation()
{
    return m_callContext.m_state == common::CallContext::cStateInProgress ? 0u : 1u;
}

unsigned int LanProcessHostMigrationJob::LanWaitSendUpdateSessionMessage()
{
    return m_callContext.m_state == common::CallContext::cStateInProgress ? 0u : 1u;
}

unsigned int LanProcessHostMigrationJob::LanCheckMatchMakingClientHost()
{
    m_state = 11;
    return m_candidate >= 0 ? 1u : 0u;
}

unsigned char LanProcessHostMigrationJob::IsFatalErrorOccur()
{
    return static_cast<unsigned char>(m_fatalError || (m_hostChanged && m_candidate < 0));
}

unsigned int LanProcessHostMigrationJob::PreprocessForSendingMigrationFinish()
{
    m_finishDeadline += 2000;
    return 1;
}

void LanProcessHostMigrationJob::Trace(unsigned long long) const
{
}

unsigned int LanProcessHostMigrationJob::StartupImpl(bool multi, StationIndex station)
{
    m_mode = multi ? 2u : 1u;
    m_multiCandidate = multi;
    m_host = station;
    m_candidate = station;
    m_hostChanged = false;
    m_fatalError = false;
    m_callContext.Reset();
    m_state = 1;
    return 1;
}

void LanProcessHostMigrationJob::CleanupImpl()
{
    m_hostChanged = false;
    m_callContext.Reset();
}

unsigned int LanProcessHostMigrationJob::StartupMultiImpl(bool multi, unsigned short candidates)
{
    m_multiCandidate = multi;
    m_candidateCount = candidates;
    m_waitLimit = candidates * 3000u + 1000u;
    m_candidate = static_cast<StationIndex>(-1);
    m_hostChanged = false;
    m_state = 1;
    return 1;
}

void LanProcessHostMigrationJob::CleanupStatus()
{
    StopRankDecisionResending();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_candidate = static_cast<StationIndex>(-1);
}

unsigned int LanProcessHostMigrationJob::CheckWhetherReselectNewHost()
{
    if (!m_multiCandidate || m_reselectDeadline == 0)
        return 0;
    if (m_waitCount >= m_reselectDeadline)
    {
        m_candidate = static_cast<StationIndex>(-1);
        m_hostChanged = false;
        StopRankDecisionResending();
        return 1;
    }
    ++m_waitCount;
    return 0;
}

unsigned int LanProcessHostMigrationJob::IsValidHostMigrationSetting()
{
    return m_mode != 0 ? 1u : 0u;
}

}}}
