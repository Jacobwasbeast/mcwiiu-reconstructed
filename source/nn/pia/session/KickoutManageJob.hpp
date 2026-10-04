#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_KICKOUT_MANAGE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_KICKOUT_MANAGE_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class KickoutManageJob : public common::StepSequenceJob
{
public:
    enum { cKickoutStationNumMax = 32 };

    enum KickoutReason
    {
        cKickoutReasonNone = 0,
        cKickoutReasonRequested = 2,
        cKickoutReasonRelayFailureBase = 3
    };

    class KickoutStation
    {
    public:
        StationIndex m_stationIndex;
        KickoutReason m_kickoutReason;
    };

    KickoutManageJob();
    virtual ~KickoutManageJob();
    virtual void Trace(unsigned long long value) const;
    virtual void SetKickoutFlag();
    virtual void KickoutFromNetwork(const common::StationAddress& stationAddress);
    bool Startup();
    void Cleanup();
    bool StartupClient(KickoutReason kickoutReason);
    Result StartKickout(StationIndex stationIndex, KickoutReason kickoutReason);
    unsigned int ClientStartLeaveMesh();
    unsigned int ClientWaitLeaveMesh();
    unsigned int ClientFinalize();
    void SetLeaveEventStationIndex(StationIndex stationIndex);
    bool AssociateKickoutWith(common::CallContext* callContext);

    void setStep(unsigned int (KickoutManageJob::*function)(), const char* name)
    {
        union
        {
            unsigned int (KickoutManageJob::*derived)();
            unsigned int (common::StepSequenceJob::*base)();
        } step;
        step.derived = function;
        m_currentStep.m_function = step.base;
        m_currentStep.m_name = name;
    }

    KickoutStation m_kickoutStations[cKickoutStationNumMax];
    KickoutReason m_kickoutReason;
    common::CallContext m_callContext;
    common::CallContext* m_kickoutCallContext;
    bool m_joinCancelRequested;
    Result m_joinCancelResult;
};

}}}

#endif
