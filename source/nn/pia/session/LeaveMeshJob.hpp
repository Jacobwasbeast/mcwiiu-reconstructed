#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_MESH_JOB_HPP
#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
namespace nn
{
namespace pia
{
namespace session
{
class LeaveMeshJob : public common::StepSequenceJob
{
public:
    LeaveMeshJob();
    virtual ~LeaveMeshJob();
    Result Startup(common::CallContext*);
    unsigned int SendLeaveRequest();
    unsigned int WaitLeaveResponse();
    unsigned int StartDisconnectStations();
    unsigned int WaitLeavingProcess();
    unsigned int LeaveSuccess();
    void Cleanup();
    unsigned int RegisterExtraCallback(common::CallContext*);
    virtual void Trace(unsigned long long) const;
    unsigned int m_deadlineHigh;
    unsigned int m_deadlineLow;
    unsigned int m_timeout;
    common::CallContext* m_callerContext;
    common::CallContext* m_extraContext;
    bool m_disconnectStarted;
};
}
}
}
#endif
