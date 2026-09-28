#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_SESSION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class LeaveSessionJob : public common::StepSequenceJob
{
public:
    LeaveSessionJob();
    virtual ~LeaveSessionJob();
    Result Startup(common::CallContext* callerContext);
    unsigned int WaitForcedTerminatingOfJointSessionJob();
    unsigned int LeaveMesh();
    unsigned int WaitLeaveMesh();
    unsigned int WaitLeaveMeshWithHostMigration();
    unsigned int WaitHostMigrated();
    unsigned int LeaveCurrentMatchmakeSession();
    unsigned int WaitLeaveCurrentMatchmakeSession();
    void CalcProcessingTimeOfLeave() const;
    unsigned int SendMonitoringData();
    unsigned int MeshCleanup();
    unsigned int CompleteProcess();
    unsigned int LeaveBufferMatchmakeSession();
    unsigned int WaitLeaveBufferMatchmakeSession();
    virtual void SetNextStepAtWaitForcedTerminatingOfJointSessionJob();
    virtual void SetNextStepAtWaitLeaveMesh();
    virtual void SetNextStepAtMeshCleanup();
    virtual void SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    virtual void SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
    virtual bool IsNeedSkipLeaveCurrentMatchmakeSession() = 0;
    virtual void SetNextStepAtWaitHostMigratedTimeout();
    virtual void SetNextStepAtWaitLeaveBufferMatchmakeSession();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetWaitHostMigrationTimeout() const;
    virtual void ChangeStatusToDisconnectedNetwork();
    virtual void CleanupImpl();

protected:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    int m_startTick;
    int m_endTick;
    bool m_isSingleStation;
    unsigned char m_padding[3];
    unsigned int m_field64;
    int m_migrationStartTick;
    int m_migrationEndTick;
    void setStep(unsigned int (LeaveSessionJob::*function)(), const char* name);
};

}}} 

#if !defined(RECON_NATIVE)
#endif

#endif
