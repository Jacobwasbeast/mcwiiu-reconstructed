#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHLEAVESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHLEAVESESSIONJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchLeaveSessionJob
{
public:
    NexMatchLeaveSessionJob();
    virtual ~NexMatchLeaveSessionJob();
    void ChooseNextStepByCheckingBufferMatchmakeSessions(unsigned long long p0);
    void CleanupImpl();
    unsigned int GetMatchmakeSessionOwners();
    unsigned int GetWaitHostMigrationTimeout();
    bool IsNeedSkipLeaveCurrentMatchmakeSession();
    unsigned int LeaveMatchmakeSessionCompanions();
    unsigned int MigrateMatchmakeSessionOwner();
    void SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
    void SetNextStepAtMeshCleanup();
    void SetNextStepAtWaitForcedTerminatingOfJointSessionJob();
    void SetNextStepAtWaitHostMigratedTimeout();
    void SetNextStepAtWaitLeaveBufferMatchmakeSession();
    void SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    void Trace(unsigned long long p0);
    unsigned int WaitGetMatchmakeSessionOwners();
    unsigned int WaitLeaveMatchmakeSessionCompanions();
    unsigned int WaitMigrateMatchmakeSessionOwner();

private:
    unsigned int m_state;
};

}}}

#endif
