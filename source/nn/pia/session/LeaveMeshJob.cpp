#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace {
using nn::pia::common::CallContext;
using nn::pia::session::Mesh;
using nn::pia::transport::DisconnectStationJob;

static unsigned int* stationCount(Mesh* mesh) {
    return reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(mesh) + 0x28);
}
static unsigned char* stationArray(Mesh* mesh) {
    return reinterpret_cast<unsigned char*>(mesh) + 0x2c;
}
static unsigned char* localStation(Mesh* mesh) {
    return *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(mesh) + 0x138);
}
static CallContext** contextAt(void* self, unsigned int offset) {
    return reinterpret_cast<CallContext**>(reinterpret_cast<unsigned char*>(self) + offset);
}
static unsigned int nowHigh() { nn::pia::common::Time now; now.SetNow(); return static_cast<unsigned int>(now.m_tick >> 32); }
static unsigned int nowLow() { nn::pia::common::Time now; now.SetNow(); return static_cast<unsigned int>(now.m_tick); }
static bool before(unsigned int ah, unsigned int al, unsigned int bh, unsigned int bl) {
    return ah < bh || (ah == bh && al < bl);
}
}

namespace nn { namespace pia { namespace session {
LeaveMeshJob::LeaveMeshJob() : common::StepSequenceJob(), m_deadlineHigh(0), m_deadlineLow(0), m_timeout(5000), m_callerContext(0), m_extraContext(0), m_disconnectStarted(false) {}
LeaveMeshJob::~LeaveMeshJob() {}
Result LeaveMeshJob::Startup(common::CallContext* context) {
    const common::Job::State state = GetState();
    if (state == common::Job::cStateRunning || state == common::Job::cStateWaiting || state == common::Job::cStateReady) return 0;
    if (context != 0) { m_callerContext = context; context->InitiateCall(); }
    const unsigned long long deadline = static_cast<unsigned long long>(nowHigh()) << 32 | nowLow();
    const unsigned long long ticks = static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * m_timeout;
    const unsigned long long end = deadline + ticks;
    m_deadlineHigh = static_cast<unsigned int>(end >> 32); m_deadlineLow = static_cast<unsigned int>(end);
    m_disconnectStarted = false;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(2));
    Reset(true);
    union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step;
    step.derived=&LeaveMeshJob::SendLeaveRequest; m_currentStep.m_function=step.base; return 1;
}
unsigned int LeaveMeshJob::SendLeaveRequest() { m_disconnectStarted = true; union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::WaitLeaveResponse; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::WaitLeaveResponse() { if (!m_disconnectStarted || !before(nowHigh(), nowLow(), m_deadlineHigh, m_deadlineLow)) m_disconnectStarted=false; union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::StartDisconnectStations; m_currentStep.m_function=step.base; return m_disconnectStarted ? cExecuteResultWait : cExecuteResultNextStep; }
unsigned int LeaveMeshJob::StartDisconnectStations() {
    Mesh* mesh = g_piaMesh; unsigned char* local = localStation(mesh); const unsigned int count = *stationCount(mesh); unsigned char* stations = stationArray(mesh);
    for (unsigned int i=0; i<count; ++i) { unsigned char* station = *reinterpret_cast<unsigned char**>(stations + i*4); if (!station || station == local) continue; const StationIndex index = *reinterpret_cast<StationIndex*>(station+0x10); if (mesh->IsValidStationIndex(index)) mesh->UnfixDisconnectedId(index); DisconnectStationJob* job = *reinterpret_cast<DisconnectStationJob**>(station+0x34); if (job) { job->Startup(reinterpret_cast<transport::Station*>(station)); job->Ready(false); } }
    union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::WaitLeavingProcess; m_currentStep.m_function=step.base; return cExecuteResultNextStep;
}
unsigned int LeaveMeshJob::WaitLeavingProcess() { Mesh* mesh=g_piaMesh; unsigned char* local=localStation(mesh); unsigned char* stations=stationArray(mesh); const unsigned int count=*stationCount(mesh); for(unsigned int i=0;i<count;++i){ unsigned char* station=*reinterpret_cast<unsigned char**>(stations+i*4); if(station && station!=local && *reinterpret_cast<unsigned int*>(station+0x20)==5) return cExecuteResultWait; } union { unsigned int (LeaveMeshJob::*derived)(); unsigned int (common::StepSequenceJob::*base)(); } step; step.derived=&LeaveMeshJob::LeaveSuccess; m_currentStep.m_function=step.base; return cExecuteResultNextStep; }
unsigned int LeaveMeshJob::LeaveSuccess() { g_piaMesh->CleanupStationsJobs(); g_piaMesh->SetMeshState(Mesh::cStateStarted); g_piaMesh->CleanupStatus(); if(m_callerContext){m_callerContext->SignalSuccess(0);m_callerContext=0;} if(m_extraContext){m_extraContext->SignalSuccess(0);m_extraContext=0;} return cExecuteResultFinished; }
void LeaveMeshJob::Cleanup() { m_deadlineHigh=0; m_deadlineLow=0; m_disconnectStarted=false; if(m_callerContext){if(m_callerContext->m_state==common::CallContext::cStateInProgress)m_callerContext->SignalSuccess(0);m_callerContext=0;} if(m_extraContext){if(m_extraContext->m_state==common::CallContext::cStateInProgress)m_extraContext->SignalSuccess(0);m_extraContext=0;} }
unsigned int LeaveMeshJob::RegisterExtraCallback(common::CallContext* context) { if(m_extraContext!=0)return 0; m_extraContext=context; if(context)context->InitiateCall(); return 1; }
void LeaveMeshJob::Trace(unsigned long long) const {}
}}}
