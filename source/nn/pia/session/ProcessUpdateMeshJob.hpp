#ifndef NN_PIA_SESSION_PROCESSUPDATEMESHJOB_HPP
#define NN_PIA_SESSION_PROCESSUPDATEMESHJOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn { namespace pia { namespace session {

class ProcessUpdateMeshJob : public common::StepSequenceJob
{
public:
    ProcessUpdateMeshJob();
    virtual ~ProcessUpdateMeshJob();
    unsigned int SetStationDataList(const unsigned char*, unsigned int);
    void CalcTimeLimit(bool);
    unsigned char GetContextIndex(StationIndex) const;
    void ClearContextIndex(StationIndex);
    unsigned short GetUsableContextIndex() const;
    void SetContextIndex(StationIndex, unsigned char);
    unsigned int UpdateMesh();
    int Startup(const unsigned char*, unsigned int, bool);
    void UpdateJoinMeshFailureResult(StationIndex, unsigned char, bool);
    unsigned int WaitDirectConnection();
    unsigned int SendConnectionReport(StationIndex, unsigned int, unsigned int*);
    unsigned int SendConnectionReport();
    unsigned int StartRelayConnection();
    unsigned int CheckConnectionAll();
    unsigned int UpdateFailed();
    unsigned int WaitLeaveMesh();
    void CleanupInternalContexts();
    unsigned int CleanupByProcessFailure();
    void UpdateDataTakeover(unsigned short);
    unsigned int UpdateStationDataList(const unsigned char*, unsigned int, bool);
    void Cleanup();
    unsigned char GetStationIndexByPrincipalID(unsigned long long) const;
    bool CheckEdmByStationIndex(StationIndex) const;
    unsigned int SetConnectionFailureNotice(StationIndex, unsigned char);
    void InitializeMonitoringDataVar();
    void SetMonitoringData();
    void ClearStationIndex(StationIndex);
    virtual void Trace(unsigned long long) const;
    void GetStepSequence() const;
    void CheckDuplicationDataInUpdateMeshDataList() const;

private:
    unsigned int m_timeLimit;
    unsigned int m_connectionDeadline;
    unsigned int m_reportDeadline;
    unsigned int m_processing;
    unsigned int m_stationCount;
    unsigned char m_contextIndex[32];
    unsigned char m_failureNotice[32];
    unsigned char m_updateStarted;
    unsigned char m_relayRequested;
    unsigned char m_monitoring;
    unsigned char m_reserved;
    unsigned short m_dataCount;
    unsigned int m_dataVersion;
    common::CallContext m_contexts[32];
};

}}}
#endif
