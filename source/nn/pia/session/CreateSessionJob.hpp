#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_SESSION_JOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/common/CallContext.hpp"

namespace nn { namespace pia { namespace session {
class CreateSessionSetting;
class CreateSessionJob : public common::StepSequenceJob
{
public:
    CreateSessionJob();
    virtual ~CreateSessionJob();
    Result Startup(common::CallContext* callerContext, const CreateSessionSetting* setting);
    unsigned int MeshStartup();
    unsigned int CreateMesh();
    unsigned int WaitCreateMesh();
    unsigned int CompleteFailure();
    unsigned int CompleteProcess();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    void SetMatchmakeStatus(Session::MatchmakeStatus status);
    void SetConnectionStatus(Session::ConnectionStatus status);
private:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    unsigned int m_result;
};
}}}

#endif
