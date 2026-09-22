#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_CREATE_MESH_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class CreateMeshJob : public common::StepSequenceJob
{
public:
    CreateMeshJob();
    virtual ~CreateMeshJob();

    void SetHostInfoToMonitoringData();
    void Startup(common::CallContext* callerContext);
    unsigned int SetupLocalPlayerInfo();
    void Cleanup();
    unsigned int CompleteFailure();
    unsigned int SetupLocalStation();

    virtual void Trace(unsigned long long value) const;
    virtual unsigned int StartupImpl();
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();
    virtual void CleanupImpl();
    virtual void SetHostInfoToMonitoringDataImpl();

    common::CallContext* m_callerContext;
};

}}}

#endif
