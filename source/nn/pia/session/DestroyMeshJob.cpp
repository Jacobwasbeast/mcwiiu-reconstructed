#include "nn/pia/session/DestroyMeshJob.hpp"

namespace nn { namespace pia { namespace session {
DestroyMeshJob::DestroyMeshJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_step(0), m_timeout(5000), m_started(false), m_systemDestroy(false), m_stationResponses() {}
DestroyMeshJob::~DestroyMeshJob() { Cleanup(); }
Result DestroyMeshJob::Startup(common::CallContext* context, bool systemDestroy) { if (m_started || context==0) return 0x10c07; m_callerContext=context; context->InitiateCall(); m_systemDestroy=systemDestroy; m_started=true; m_step=0; for(unsigned int i=0;i<sizeof(m_stationResponses);++i)m_stationResponses[i]=1; Reset(true); union { unsigned int (DestroyMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&DestroyMeshJob::SendDestroyMesh; m_currentStep.m_function=step.base; return 0; }
unsigned int DestroyMeshJob::AssociateSystemWith(common::CallContext* context) { if (!m_started || !m_systemDestroy || context==0) return 0x10c07; m_callerContext=context; context->InitiateCall(); m_systemDestroy=false; return 0; }
unsigned int DestroyMeshJob::SendDestroyMesh() { union { unsigned int (DestroyMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&DestroyMeshJob::WaitDestroyResponse; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int DestroyMeshJob::WaitDestroyResponse() { for(unsigned int i=0;i<sizeof(m_stationResponses);++i) if(m_stationResponses[i] != 0) return cExecuteResultWait; union { unsigned int (DestroyMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&DestroyMeshJob::CleanupMesh; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
void DestroyMeshJob::CleanupStatus() { m_started=false; m_systemDestroy=false; m_step=0; for(unsigned int i=0;i<sizeof(m_stationResponses);++i)m_stationResponses[i]=0; }
unsigned int DestroyMeshJob::CleanupMesh() { if (m_callerContext) { m_callerContext->SignalSuccess(0); m_callerContext=0; } CleanupStatus(); return cExecuteResultFinished; }
void DestroyMeshJob::ReceiveDestroyResponse(StationIndex index) { if (index>=0 && static_cast<unsigned int>(index)<sizeof(m_stationResponses)) m_stationResponses[index]=0; }
void DestroyMeshJob::Cleanup() { if (m_callerContext) { if (m_callerContext->m_state==common::CallContext::cStateInProgress)m_callerContext->SignalCancel(); m_callerContext=0; } if (m_callContext.m_state==common::CallContext::cStateInProgress)m_callContext.SignalCancel(); m_callContext.Reset(); CleanupStatus(); }
void DestroyMeshJob::Trace(unsigned long long) const {}
}}}
