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

    class KickoutStation
    {
    public:
        StationIndex m_stationIndex;
        unsigned int m_kickoutReason;
    };

    KickoutManageJob();
    virtual ~KickoutManageJob();
    virtual void Trace(unsigned long long value) const;
    virtual void SetKickoutFlag();
    virtual void KickoutFromNetwork(const common::StationAddress& stationAddress);
    bool Startup();
    void Cleanup();
    Result StartupClient(unsigned int kickoutReason);
    Result StartKickout(StationIndex stationIndex, unsigned int kickoutReason);
    unsigned int ClientStartLeaveMesh();
    unsigned int ClientWaitLeaveMesh();
    unsigned int ClientFinalize();
    void SetLeaveEventStationIndex(StationIndex stationIndex);
    bool AssociateKickoutWith(common::CallContext* callContext);

    KickoutStation m_kickoutStations[cKickoutStationNumMax];
    unsigned int m_kickoutReason;
    common::CallContext m_callContext;
    common::CallContext* m_kickoutCallContext;
    bool m_joinCancelRequested;
    Result m_joinCancelResult;
};

}}}

#endif
