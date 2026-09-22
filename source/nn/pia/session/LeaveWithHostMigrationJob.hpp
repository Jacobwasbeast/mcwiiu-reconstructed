#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class LeaveWithHostMigrationJob : public common::StepSequenceJob
{
public:
    LeaveWithHostMigrationJob();
    virtual ~LeaveWithHostMigrationJob();

    Result Startup(common::CallContext* context);
    unsigned int SendStartMigrationMessage();
    unsigned int WaitMigrationResponse();
    unsigned int CleanupMesh();
    unsigned int SendStartMultiMigrationMessage();
    unsigned int CheckHostMigrationProcess();
    unsigned int WaitHostMigrationProcess();
    unsigned int StartDisconnectStations();
    unsigned int WaitDisconnectStations();
    void ReceiveMigrationResponse(StationIndex station);
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    void CleanupStatus();

private:
    common::CallContext* m_callerContext;
    unsigned int m_deadlineHigh;
    unsigned int m_deadlineLow;
    unsigned int m_timeoutMsec;
    bool m_started;
    unsigned int m_newHost;
    unsigned char m_stationResponses[32];
    bool m_multiMigration;
};

}}}

#endif
