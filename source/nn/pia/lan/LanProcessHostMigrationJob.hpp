#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_PROCESS_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_PROCESS_HOST_MIGRATION_JOB_HPP

#include <cstdint>

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn { namespace pia { namespace lan {

class LanProcessHostMigrationJob
{
public:
    LanProcessHostMigrationJob();
    virtual ~LanProcessHostMigrationJob();

    unsigned int LanCleanupOldHostInfo();
    unsigned int LanDecideNextHost();
    unsigned int LanPrepareForBecomingHost();
    unsigned int LanCleanupOldHostInfoOnMultiCandidate();
    unsigned int LanMakeHostCandidateRanking();
    int CalcTimeToWaitOtherClients(unsigned int rank, unsigned int count);
    unsigned int LanSendRankDecision();
    unsigned int LanWaitRankDecision();
    unsigned int LanGetMatchMakingClientHost();
    unsigned int LanWaitMatchMakingClientHost();
    unsigned int LanCheckOldHostDisconnection();
    unsigned int LanWaitCheckOldHostDisconnection();
    void StopRankDecisionResending();
    unsigned int LanPrepareForBecomingHostMulti();
    unsigned int LanGetMatchMakingClientHostLastConfirmation();
    unsigned int LanWaitMatchMakingClientHostLastConfirmation();
    unsigned int LanWaitSendUpdateSessionMessage();
    unsigned int LanCheckMatchMakingClientHost();
    unsigned char IsFatalErrorOccur();
    unsigned int PreprocessForSendingMigrationFinish();
    void Trace(unsigned long long value) const;
    unsigned int StartupImpl(bool multi, StationIndex station);
    void CleanupImpl();
    unsigned int StartupMultiImpl(bool multi, unsigned short candidates);
    void CleanupStatus();
    unsigned int CheckWhetherReselectNewHost();
    unsigned int IsValidHostMigrationSetting();

private:
    common::CallContext m_callContext;
    unsigned int m_state;
    unsigned int m_mode;
    StationIndex m_host;
    StationIndex m_candidate;
    unsigned int m_rank;
    unsigned int m_candidateCount;
    unsigned int m_waitCount;
    unsigned int m_waitLimit;
    bool m_multiCandidate;
    bool m_hostChanged;
    bool m_fatalError;
    unsigned int m_finishDeadline;
    unsigned int m_reselectDeadline;
};

}}}

#endif
