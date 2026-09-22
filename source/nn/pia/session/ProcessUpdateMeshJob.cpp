#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include <cstring>

namespace nn { namespace pia { namespace session {

ProcessUpdateMeshJob::ProcessUpdateMeshJob()
    : common::StepSequenceJob(), m_timeLimit(0), m_connectionDeadline(0),
      m_reportDeadline(0), m_processing(0), m_stationCount(0),
      m_updateStarted(0), m_relayRequested(0), m_monitoring(0), m_reserved(0),
      m_dataCount(0), m_dataVersion(0)
{
    for (unsigned int i = 0; i < 32; ++i) { m_contextIndex[i] = 0xff; m_failureNotice[i] = 0; }
}

ProcessUpdateMeshJob::~ProcessUpdateMeshJob() { CleanupInternalContexts(); }

unsigned int ProcessUpdateMeshJob::SetStationDataList(const unsigned char* data, unsigned int size)
{
    m_dataCount = static_cast<unsigned short>(size); m_dataVersion = data ? data[0] : 0; return data ? 4 : 0;
}
void ProcessUpdateMeshJob::CalcTimeLimit(bool retry)
{ m_timeLimit = retry ? 0 : 25000; m_connectionDeadline = m_timeLimit; m_reportDeadline = m_timeLimit; }
unsigned char ProcessUpdateMeshJob::GetContextIndex(StationIndex station) const
{ return (station >= 0 && station < 32) ? m_contextIndex[station] : 0xff; }
void ProcessUpdateMeshJob::ClearContextIndex(StationIndex station)
{ if (station >= 0 && station < 32) m_contextIndex[station] = 0xff; }
unsigned short ProcessUpdateMeshJob::GetUsableContextIndex() const
{ for (unsigned short i = 0; i < 32; ++i) { bool used = false; for (unsigned int j=0;j<32;++j) if (m_contextIndex[j] == i) used=true; if (!used) return i; } return 0xffff; }
void ProcessUpdateMeshJob::SetContextIndex(StationIndex station, unsigned char context)
{ if (station >= 0 && station < 32) m_contextIndex[station] = context; }
unsigned int ProcessUpdateMeshJob::UpdateMesh() { m_updateStarted = 1; return 0; }
int ProcessUpdateMeshJob::Startup(const unsigned char* data, unsigned int size, bool relay)
{ unsigned int result=SetStationDataList(data,size); if (result==4) { m_relayRequested=relay?1:0; CalcTimeLimit(relay); m_processing=1; } return static_cast<int>(result); }
void ProcessUpdateMeshJob::UpdateJoinMeshFailureResult(StationIndex station, unsigned char reason, bool retry)
{ if (station>=0 && station<32) m_failureNotice[station]=reason; if (!retry) m_processing=0; }
unsigned int ProcessUpdateMeshJob::WaitDirectConnection() { return m_processing ? 0x1000000u : 0x5000000u; }
unsigned int ProcessUpdateMeshJob::SendConnectionReport(StationIndex station, unsigned int value, unsigned int* output)
{ if (output) *output=value; if (station>=0 && station<32) m_failureNotice[station]=0; return 0; }
unsigned int ProcessUpdateMeshJob::SendConnectionReport() { return SendConnectionReport(-1,0,0); }
unsigned int ProcessUpdateMeshJob::StartRelayConnection() { m_relayRequested=1; return 0; }
unsigned int ProcessUpdateMeshJob::CheckConnectionAll() { return m_processing ? 0 : 1; }
unsigned int ProcessUpdateMeshJob::UpdateFailed() { m_processing=0; return 0; }
unsigned int ProcessUpdateMeshJob::WaitLeaveMesh() { return m_processing ? 0x1000000u : 0x5000000u; }
void ProcessUpdateMeshJob::CleanupInternalContexts() { for (unsigned int i=0;i<32;++i) m_contexts[i].Reset(); }
unsigned int ProcessUpdateMeshJob::CleanupByProcessFailure() { Cleanup(); return 0; }
void ProcessUpdateMeshJob::UpdateDataTakeover(unsigned short count) { m_dataCount=count; }
unsigned int ProcessUpdateMeshJob::UpdateStationDataList(const unsigned char* data, unsigned int size, bool relay)
{ int result=Startup(data,size,relay); return result==4 ? UpdateMesh() : static_cast<unsigned int>(result); }
void ProcessUpdateMeshJob::Cleanup()
{ CleanupInternalContexts(); m_processing=0; m_stationCount=0; m_dataCount=0; m_updateStarted=0; m_relayRequested=0; InitializeMonitoringDataVar(); }
unsigned char ProcessUpdateMeshJob::GetStationIndexByPrincipalID(unsigned long long) const { return 0xff; }
bool ProcessUpdateMeshJob::CheckEdmByStationIndex(StationIndex station) const { return GetContextIndex(station) != 0xff; }
unsigned int ProcessUpdateMeshJob::SetConnectionFailureNotice(StationIndex station, unsigned char value)
{ if (station<0 || station>=32) return 0; m_failureNotice[station]=value; return 0; }
void ProcessUpdateMeshJob::InitializeMonitoringDataVar() { m_monitoring=0; }
void ProcessUpdateMeshJob::SetMonitoringData() { m_monitoring=1; }
void ProcessUpdateMeshJob::ClearStationIndex(StationIndex station) { ClearContextIndex(station); }
void ProcessUpdateMeshJob::Trace(unsigned long long) const {}
void ProcessUpdateMeshJob::GetStepSequence() const {}
void ProcessUpdateMeshJob::CheckDuplicationDataInUpdateMeshDataList() const {}

}}}
