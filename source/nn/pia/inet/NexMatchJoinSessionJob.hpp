#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINSESSIONJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchJoinSessionJob
{
public:
    NexMatchJoinSessionJob();
    virtual ~NexMatchJoinSessionJob();
    unsigned int CleanupForRetryJoinMesh();
    void CleanupImpl();
    unsigned int GetStationConnectionInfo();
    unsigned int JoinMatchmakeSession();
    unsigned int LeaveJoinedMatchmakeSession();
    unsigned int LeaveMatchmakeSession();
    unsigned long long NoticeOwnerChangeEvent(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void NoticeUnregisterEvent(unsigned long long p0);
    unsigned int SetNextStepAtFailedJoinMesh(unsigned long long p0);
    unsigned int SetNextStepAtMeshCleanup();
    void SetNextStepAtWaitJoinMesh();
    unsigned int SetNextStepAtWaitLeaveMesh();
    unsigned int StartNatSession();
    unsigned long long StartupImpl(unsigned long long p0, unsigned long long p1);
    void Trace(unsigned long long p0);
    unsigned int WaitForRetryJoinMesh();
    unsigned int WaitGetStationConnectionInfo();
    unsigned int WaitJoinMatchmake();
    unsigned int WaitLeaveBufferMatchmakeSession();
    unsigned int WaitLeaveCurrentMatchmakeSession();
    unsigned int WaitLeaveJoinedMatchmakeSession();
    unsigned int WaitNotification();
    unsigned int WaitStartNatSession();

private:
    unsigned int m_state;
};

}}}

#endif
