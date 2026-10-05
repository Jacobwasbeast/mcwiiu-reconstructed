#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RANDOM_MATCHMAKE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RANDOM_MATCHMAKE_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace session {

class JoinRandomSessionSetting;

class RandomMatchmakeJob : public common::StepSequenceJob
{
public:
    RandomMatchmakeJob();
    virtual ~RandomMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void Cleanup();
    virtual unsigned int GetJoinSessionStateForDebug() const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    virtual Result StartupImpl(const JoinRandomSessionSetting* setting) = 0;
    virtual void CleanupImpl();
    virtual void SetNextStepAtMeshStartup();
    virtual void SetNextStepAtWaitCreateMesh() = 0;
    virtual void SetNextStepAtWaitJoinMesh() = 0;
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result) = 0;
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result) = 0;
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh() = 0;

    Result Startup(common::CallContext* callerContext, const JoinRandomSessionSetting* setting);
    ExecuteResult MeshStartup();
    ExecuteResult CreateMesh();
    ExecuteResult WaitCreateMesh();
    ExecuteResult GetStationConnection();
    ExecuteResult WaitGetStationConnection();
    ExecuteResult JoinMesh();
    bool CheckContextCallCancelled();
    ExecuteResult WaitJoinMesh();
    ExecuteResult LeaveMesh();
    ExecuteResult WaitLeaveMesh();
    void CalcProcessingTimeOfJoin() const;
    ExecuteResult CompleteProcess();
    ExecuteResult CompleteFailure();
    void CleanupInternalContexts();
    void SetMatchmakeStatus(Session::MatchmakeStatus status);
    void SetConnectionStatus(Session::ConnectionStatus status);

    void setStep(ExecuteResult (RandomMatchmakeJob::*function)(), const char* name)
    {
        union
        {
            ExecuteResult (RandomMatchmakeJob::*derived)();
            ExecuteResult (common::StepSequenceJob::*base)();
        } step;
        step.derived = function;
        m_currentStep.m_function = step.base;
        m_currentStep.m_name = name;
    }

    common::CallContext* m_callerContext;
    unsigned int m_joinState;
    unsigned int m_unknown48;
    bool m_unknown4c;
    bool m_unknown4d;
    Result m_result;
    bool m_unknown54;
    bool m_unknown55;
    bool m_unknown56;
    bool m_unknown57;
    bool m_unknown58;
    bool m_unknown59;
    bool m_unknown5a;
    unsigned long long m_principalId;
    transport::StationConnectionInfo m_stationConnectionInfo;
    common::CallContext m_callContext;
    common::Time m_startTime;
};

}}}

#endif
