#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_SESSION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class DestroySessionJob : public common::StepSequenceJob
{
public:
    DestroySessionJob();
    virtual ~DestroySessionJob();
    Result Startup(common::CallContext* callerContext);
    unsigned int WaitForcedTerminatingOfJointSessionJob();
    unsigned int DestroyMesh();
    unsigned int WaitDestroyMesh();
    unsigned int MeshCleanup();
    void CalcProcessingTimeOfLeave() const;
    unsigned int SendMonitoringData();
    unsigned int CompleteProcess();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    int m_startTick;
    int m_endTick;
    bool m_isSingleStation;
    unsigned char m_padding[7];
    virtual void SetNextStepAtWaitDestroyMesh() = 0;
    virtual void SetNextStepAtMeshCleanup() = 0;
    void setStep(unsigned int (DestroySessionJob::*function)(), const char* name);
};

}}} 

static_assert(sizeof(nn::pia::session::DestroySessionJob) == 0x68, "DestroySessionJob ABI");

#endif
