#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINTSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINTSESSIONJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchJointSessionJob
{
public:
    NexMatchJointSessionJob();
    virtual ~NexMatchJointSessionJob();
    unsigned int CallDestroySessionEvent();
    unsigned int CallSessionEvent();
    void CheckJoinedOnlyCompanion();
    void CleanupImpl();
    void CleanupMonitoringData();
    unsigned int CloseJointSessionParticipation();
    unsigned int CloseMatchmakeSession();
    unsigned int GetJointSessionStateForDebug();
    unsigned int GetMatchmakeSessionOwners();
    unsigned int GetNextMatchmakeSessionInfo();
    unsigned int MeshRestart();
    unsigned long long NoticeHostChangeEventImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long NoticeLeaveEventImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long NoticeOwnerChangeEventImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void NoticeUnregisterEventImpl(unsigned long long p0);
    void PrepareBecomingToLeader();
    unsigned int ProcessComplete();
    unsigned int ProcessFailure();
    void ReceiveCompletionResponseImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long ReceiveLeaveInvitationImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5, unsigned long long p6);
    void ReceiveLeaveMeshInvitationImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long ReceiveNextSessionIdImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned int ResendDestroyInvitation();
    unsigned int ResendInvitationAsCompanion();
    unsigned int ResendNextSessionId();
    unsigned int SendAnswerToDestroyInvitation();
    unsigned int SendAnswerToInvitation();
    unsigned int SendCompletionForMigrateSession();
    unsigned int SendCompletionInvitation();
    unsigned int SendDestroyInvitation();
    unsigned int SendInvitationAsCompanion();
    unsigned int SendLeaveMeshCompanion();
    unsigned int SendNextSessionId();
    unsigned int SendPreparedForMigrateSession();
    unsigned int StartCreateMatchmakeSession();
    unsigned int StartCreateNextMesh();
    unsigned int StartGetNextMeshHostStationConnectionInfo();
    unsigned int StartJoinMatchmakeSession();
    unsigned int StartJoinNextMesh();
    unsigned int StartLeaveBufferMatchmakeSession();
    unsigned int StartLeaveCurrentMatchmakeSession();
    unsigned int StartLeaveMesh();
    unsigned int StartLeavePreviousMatchmakeSession();
    unsigned int StartLeavePreviousMesh();
    unsigned int StartRandomMatchmake();
    unsigned long long StartupCreateJointSessionImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long StartupDestroyJointSessionImpl();
    unsigned long long StartupJoinJointSessionImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long StartupJoinRandomJointSessionImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long StartupLeaveJointSessionImpl();
    unsigned long long StartupPassiveProcessImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5, unsigned long long p6);
    void Trace(unsigned long long p0);
    unsigned int UpdateJointSessionOpenStatus();
    unsigned int WaitCloseJointSessionParticipation();
    unsigned int WaitCloseMatchmakeSession();
    unsigned int WaitCompanionStation();
    unsigned int WaitCompanionStationPrepared();
    unsigned int WaitCompletionInvitation();
    unsigned int WaitCreateMatchmakeSession();
    unsigned int WaitCreateNextMesh();
    unsigned int WaitForAnswerToCompletionInvitation();
    unsigned int WaitForAnswerToDestroyInvitation();
    unsigned int WaitForAnswerToInvitation();
    unsigned int WaitForInvitationAsCompanion();
    unsigned int WaitGetMatchmakeSessionOwners();
    unsigned int WaitGetNextMatchmakeSessionInfo();
    unsigned int WaitGetNextMeshHostStationConnectionInfo();
    unsigned int WaitHostMigrated();
    unsigned int WaitHostStationId();
    unsigned int WaitJoinMatchmakeSession();
    unsigned int WaitJoinNextMesh();
    unsigned int WaitLeaveBufferMatchmakeSession();
    unsigned int WaitLeaveCurrentMatchmakeSession();
    unsigned int WaitLeaveMesh();
    unsigned int WaitLeavePreviousMatchmakeSession();
    unsigned int WaitLeavePreviousMesh();
    unsigned int WaitMeshRestart();
    unsigned int WaitNextSessionId();
    unsigned int WaitNotification();
    unsigned int WaitRandomMatchmake();
    unsigned int WaitStartRetryJoinMesh();
    unsigned int WaitUntilCompanionCompletion();
    unsigned int WaitUntilCompanionLeavesMesh();
    unsigned int WaitUntilLeaderLeavesMesh();
    unsigned int WaitUpdateJointSessionOpenStatus();

private:
    unsigned int m_state;
};

}}}

#endif
