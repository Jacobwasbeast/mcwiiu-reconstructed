#include "nn/pia/session/LeaveMeshJob.hpp"

namespace nn { namespace pia { namespace session {
LeaveMeshJob::LeaveMeshJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_step(0), m_timeout(5000), m_disconnectStarted(false) {}
LeaveMeshJob::~LeaveMeshJob() { Cleanup(); }
Result LeaveMeshJob::Startup(common::CallContext* context) { if (context == 0 || m_callerContext != 0) return 0x10c07; m_callerContext=context; context->InitiateCall(); m_step=0; m_disconnectStarted=false; Reset(true); union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::SendLeaveRequest; m_currentStep.m_function=step.base; return 0; }
unsigned int LeaveMeshJob::SendLeaveRequest() { union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::WaitLeaveResponse; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::WaitLeaveResponse() { union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::StartDisconnectStations; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::StartDisconnectStations() { m_disconnectStarted=true; union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::WaitLeavingProcess; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::WaitLeavingProcess() { union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::LeaveSuccess; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::LeaveSuccess() { if (m_callerContext) { m_callerContext->SignalSuccess(0); m_callerContext=0; } return cExecuteResultFinished; }
void LeaveMeshJob::Cleanup() { if (m_callerContext) { if (m_callerContext->m_state==common::CallContext::cStateInProgress) m_callerContext->SignalCancel(); m_callerContext=0; } if (m_callContext.m_state==common::CallContext::cStateInProgress) m_callContext.SignalCancel(); m_callContext.Reset(); m_step=0; }
unsigned int LeaveMeshJob::RegisterExtraCallback(common::CallContext* context) { if (context==0) return 0x10c07; m_callContext=*context; return 0; }
void LeaveMeshJob::Trace(unsigned long long) const {}
}}}
