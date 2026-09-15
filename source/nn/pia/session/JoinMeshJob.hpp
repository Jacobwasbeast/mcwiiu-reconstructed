#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_MESH_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace session {

class JoinMeshJob : public common::StepSequenceJob
{
public:
    JoinMeshJob();
    virtual ~JoinMeshJob();
    Result Startup(const transport::StationConnectionInfo&, common::CallContext*);
    unsigned int SetupLocalPlayerInfo();
    unsigned int SetupLocalStation();
    void CleanupStations();
    void CalcProcessingTimeOfJoin() const;
    bool CheckRelayConnectionForMonitoring() const;
    unsigned int CheckContextCallCancelled();
    unsigned int StartConnectingToHost();
    unsigned int CheckTransportConnectionStatus();
    unsigned int WaitUntilConnectToHost();
    unsigned int SendJoinRequest();
    unsigned int CheckConnectionStateWithHostStation();
    unsigned int WaitRequestAck();
    unsigned int WaitJoinResponse();
    unsigned int UpdateLocalAndHostInformation(StationIndex, StationIndex);
    unsigned int SetStationDataList(const unsigned char*, unsigned int);
    unsigned int AnalyzeJoinResponse();
    unsigned int WaitAllConnection();
    unsigned int CompleteProcess();
    unsigned int SetupLocalPlayerInfoFailure();
    unsigned int StartDisconnectStations();
    unsigned int WaitDisconnectStations();
    unsigned int LeaveMeshWithHostMigration();
    unsigned int LeaveMesh();
    unsigned int WaitLeaveMesh();
    unsigned int CompleteCancelWithLeaveMesh();
    void Cleanup(Result);
    unsigned int GetJoinMeshState() const;
    virtual void Trace(unsigned long long) const;
    virtual unsigned int StartupImpl();
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();
    virtual void CleanupImpl();
    virtual void SetHostInfoToMonitoringData(const transport::StationConnectionInfo&);
    virtual unsigned int SetNextStepAfterWaitAllConnection();
    void CleanupInternalContexts();

    common::CallContext* m_callerContext;
    common::CallContext* m_internalContext;
    unsigned int m_status;
    StationIndex m_localStationIndex;
    StationIndex m_hostStationIndex;
    const transport::StationConnectionInfo* m_stationConnectionInfo;
    const unsigned char* m_stationData;
    unsigned char m_joinState;
    unsigned char m_cancelWithLeave;
    unsigned char m_hostMigration;
    unsigned char m_stationDataSize;
    unsigned int m_step;
    unsigned int m_result;
    unsigned int m_sessionId;
    unsigned int m_stationBitmap;
    unsigned int m_hostBitmap;
    unsigned int m_startTick;
    unsigned int m_deadlineTick;
    unsigned int m_field88;
    unsigned char m_padding7c[0x14];
    common::CallContext m_callContext;
};

}}}
#endif
