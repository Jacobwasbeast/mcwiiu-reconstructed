#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn { namespace pia { namespace session {

JoinMeshJob::JoinMeshJob() : common::StepSequenceJob(), m_callerContext(0), m_internalContext(0), m_status(0), m_localStationIndex(-1), m_hostStationIndex(-1), m_stationConnectionInfo(0), m_stationData(0), m_joinState(0), m_cancelWithLeave(false), m_hostMigration(false), m_stationDataSize(0), m_step(0), m_result(0), m_sessionId(0), m_stationBitmap(0), m_hostBitmap(0), m_startTick(0), m_deadlineTick(0), m_field88(0), m_callContext() {}
JoinMeshJob::~JoinMeshJob() { CleanupInternalContexts(); }

Result JoinMeshJob::Startup(const transport::StationConnectionInfo& info, common::CallContext* context)
{
    if (context == 0 || m_callerContext != 0) return 0x10c07;
    m_stationConnectionInfo = &info; m_callerContext = context; context->InitiateCall();
    m_result = 0; m_joinState = 1; m_step = 0; m_cancelWithLeave = false; m_hostMigration = false;
    Reset(true);
    union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step;
    step.derived = &JoinMeshJob::SetupLocalPlayerInfo; m_currentStep.m_function = step.base;
    return 0;
}

unsigned int JoinMeshJob::SetupLocalPlayerInfo() { m_joinState = 2; SetNextStepAtSetupLocalPlayerInfoImpl(); return cExecuteResultContinue; }
unsigned int JoinMeshJob::SetupLocalStation() { m_joinState = 3; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::StartConnectingToHost; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
void JoinMeshJob::CleanupStations() { m_step = 0; }
void JoinMeshJob::CalcProcessingTimeOfJoin() const {}
bool JoinMeshJob::CheckRelayConnectionForMonitoring() const { return false; }
unsigned int JoinMeshJob::CheckContextCallCancelled() { return m_callerContext != 0 && m_callerContext->m_cancelRequested ? cExecuteResultCancelled : cExecuteResultContinue; }
unsigned int JoinMeshJob::StartConnectingToHost() { m_joinState = 4; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::WaitUntilConnectToHost; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::CheckTransportConnectionStatus() { return cExecuteResultContinue; }
unsigned int JoinMeshJob::WaitUntilConnectToHost() { m_joinState = 5; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::SendJoinRequest; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::SendJoinRequest() { m_joinState = 6; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::WaitRequestAck; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::CheckConnectionStateWithHostStation() { return cExecuteResultContinue; }
unsigned int JoinMeshJob::WaitRequestAck() { m_joinState = 7; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::WaitJoinResponse; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::WaitJoinResponse() { m_joinState = 8; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::AnalyzeJoinResponse; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::UpdateLocalAndHostInformation(StationIndex, StationIndex) { return cExecuteResultContinue; }
unsigned int JoinMeshJob::SetStationDataList(const unsigned char* data, unsigned int size) { m_stationData = data; m_stationDataSize = static_cast<unsigned char>(size > 0xffu ? 0xffu : size); return cExecuteResultContinue; }
unsigned int JoinMeshJob::AnalyzeJoinResponse() { m_joinState = 9; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::WaitAllConnection; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::WaitAllConnection() { m_joinState = 10; union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&JoinMeshJob::CompleteProcess; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::CompleteProcess() { m_joinState = 0; m_result = 0; if (m_callerContext) { m_callerContext->SignalSuccess(m_result); m_callerContext = 0; } return cExecuteResultFinished; }
unsigned int JoinMeshJob::SetupLocalPlayerInfoFailure() { m_result = 0x6c50; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::StartDisconnectStations() { m_step = 0; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::WaitDisconnectStations() { return cExecuteResultNextStep; }
unsigned int JoinMeshJob::LeaveMeshWithHostMigration() { m_hostMigration = true; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::LeaveMesh() { m_cancelWithLeave = true; return cExecuteResultNextStep; }
unsigned int JoinMeshJob::WaitLeaveMesh() { return CompleteCancelWithLeaveMesh(); }
unsigned int JoinMeshJob::CompleteCancelWithLeaveMesh() { Cleanup(m_result); return cExecuteResultCancelled; }
void JoinMeshJob::Cleanup(Result result) { m_result = result; CleanupImpl(); if (m_callerContext) { if (result) m_callerContext->SignalFailure(result); else m_callerContext->SignalSuccess(result); m_callerContext = 0; } CleanupInternalContexts(); m_joinState = 0; }
unsigned int JoinMeshJob::GetJoinMeshState() const { return m_joinState; }
void JoinMeshJob::Trace(unsigned long long) const {}
unsigned int JoinMeshJob::StartupImpl() { return cExecuteResultContinue; }
void JoinMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl() { union { unsigned int (JoinMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived = &JoinMeshJob::SetupLocalStation; m_currentStep.m_function = step.base; }
void JoinMeshJob::CleanupImpl() {}
void JoinMeshJob::SetHostInfoToMonitoringData(const transport::StationConnectionInfo&) {}
unsigned int JoinMeshJob::SetNextStepAfterWaitAllConnection() { return cExecuteResultContinue; }
void JoinMeshJob::CleanupInternalContexts() { if (m_callContext.m_state == common::CallContext::cStateInProgress) m_callContext.SignalCancel(); m_callContext.Reset(); }

}}}
