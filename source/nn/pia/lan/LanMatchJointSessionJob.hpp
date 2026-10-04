#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP

#include "nn/pia/session/JointSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchJointSessionJob : public session::JointSessionJob
{
public:
    LanMatchJointSessionJob();
    virtual ~LanMatchJointSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetJointSessionStateForDebug() const;
    virtual Result StartupJoinRandomJointSessionImpl(
        const session::JoinRandomSessionSetting* setting);
    virtual Result StartupCreateJointSessionImpl(const session::CreateSessionSetting* setting);
    virtual Result StartupJoinJointSessionImpl(const session::JoinSessionSetting* setting);
    virtual Result StartupLeaveJointSessionImpl();
    virtual Result StartupDestroyJointSessionImpl();
    virtual Result StartupPassiveProcessImpl(session::Session::ProcessType processType,
                                             unsigned int messageType,
                                             const unsigned long long* stationIds,
                                             unsigned short stationIdNum,
                                             unsigned long long senderStationId);
    virtual Result ReceiveNextSessionIdImpl(session::Session::ProcessType processType,
                                            unsigned int sessionId,
                                            const unsigned char* applicationData,
                                            unsigned char applicationDataSize,
                                            unsigned long long senderStationId);
    virtual Result ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                              const unsigned long long* stationIds,
                                              unsigned short stationIdNum,
                                              unsigned long long senderStationId);
    virtual Result ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                  unsigned long long senderStationId);
    virtual Result ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                 unsigned long long senderStationId);
    virtual void CleanupImpl();

    unsigned int CallSessionEvent();
    unsigned int SendInvitationAsCompanion();
    unsigned int WaitForAnswerToInvitation();
    unsigned int StartRandomMatchmake();
    unsigned int WaitRandomMatchmake();
    unsigned int StartJoinMatchmakeSession();
    unsigned int WaitJoinMatchmakeSession();
    unsigned int SendNextSessionId();
    unsigned int WaitCompanionStationPrepared();
    unsigned int SendAnswerToInvitation();
    unsigned int WaitNextSessionId();
    unsigned int StartUpdateSessionKey();
    unsigned int WaitUpdateSessionKey();
    unsigned int SendPreparedForMigrateSession();
    unsigned int WaitUntilLeaderLeavesMesh();
    unsigned int StartLeavePreviousMesh();
    unsigned int WaitLeavePreviousMesh();
    unsigned int StartLeavePreviousMatchmakeSession();
    unsigned int WaitLeavePreviousMatchmakeSession();
    unsigned int MeshRestart();
    unsigned int StartCreateNextMesh();
    unsigned int WaitCreateNextMesh();
    unsigned int StartGetNextMeshHostStationConnectionInfo();
    unsigned int WaitGetNextMeshHostStationConnectionInfo();
    unsigned int StartJoinNextMesh();
    unsigned int WaitJoinNextMesh();
    unsigned int WaitCompanionStation();
    unsigned int WaitHostStationId();
    unsigned int ProcessComplete();

private:
    unsigned char m_reserved[120];
};

}
}
}

#endif
