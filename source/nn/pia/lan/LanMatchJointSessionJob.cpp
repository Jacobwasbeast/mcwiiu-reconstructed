#include "nn/pia/lan/LanMatchJointSessionJob.hpp"

namespace nn { namespace pia { namespace lan {
namespace {
typedef unsigned int (LanMatchJointSessionJob::*Step)();
inline void SelectStep(common::StepSequenceJob::Step& slot, Step step)
{
    union { Step derived; unsigned int (common::StepSequenceJob::*base)(); } value;
    value.derived = step;
    slot.m_function = value.base;
}
}

LanMatchJointSessionJob::LanMatchJointSessionJob()
    : session::JointSessionJob()
{
}

LanMatchJointSessionJob::~LanMatchJointSessionJob()
{
}

unsigned int LanMatchJointSessionJob::CallSessionEvent()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::SendInvitationAsCompanion()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitForAnswerToInvitation()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartRandomMatchmake()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitRandomMatchmake()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartJoinMatchmakeSession()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::SendNextSessionId()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitCompanionStationPrepared()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::SendAnswerToInvitation()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitNextSessionId()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartUpdateSessionKey()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitUpdateSessionKey()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::SendPreparedForMigrateSession()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartLeavePreviousMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitLeavePreviousMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::MeshRestart()
{
    if (m_field9d == 0)
    {
        SelectStep(m_currentStep, &LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo);
        m_currentStep.m_name = "LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo";
    }
    else
    {
        SelectStep(m_currentStep, &LanMatchJointSessionJob::StartCreateNextMesh);
        m_currentStep.m_name = "LanMatchJointSessionJob::StartCreateNextMesh";
    }
    return cExecuteResultContinue;
}

unsigned int LanMatchJointSessionJob::StartCreateNextMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitCreateNextMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::StartJoinNextMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitJoinNextMesh()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitCompanionStation()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::WaitHostStationId()
{
    return 0;
}

unsigned int LanMatchJointSessionJob::ProcessComplete()
{
    return 0;
}

void LanMatchJointSessionJob::Trace(unsigned long long value) const
{
}

Result LanMatchJointSessionJob::StartupJoinRandomJointSessionImpl(
    const session::JoinRandomSessionSetting* setting)
{
    return 0;
}

Result LanMatchJointSessionJob::StartupCreateJointSessionImpl(
    const session::CreateSessionSetting* setting)
{
    return 0;
}

Result LanMatchJointSessionJob::StartupJoinJointSessionImpl(
    const session::JoinSessionSetting* setting)
{
    return 0;
}

Result LanMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    return 0;
}

Result LanMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    return 0;
}

Result LanMatchJointSessionJob::StartupPassiveProcessImpl(session::Session::ProcessType processType,
                                                          unsigned int messageType,
                                                          const unsigned long long* stationIds,
                                                          unsigned short stationIdNum,
                                                          unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                                           const unsigned long long* stationIds,
                                                           unsigned short stationIdNum,
                                                           unsigned long long senderStationId)
{
    return 0;
}

unsigned int LanMatchJointSessionJob::GetJointSessionStateForDebug() const
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveNextSessionIdImpl(session::Session::ProcessType processType,
                                                         unsigned int sessionId,
                                                         const unsigned char* applicationData,
                                                         unsigned char applicationDataSize,
                                                         unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                               unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                              unsigned long long senderStationId)
{
    return 0;
}

void LanMatchJointSessionJob::CleanupImpl()
{
}

}}}
