#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn { namespace pia { namespace session {

class LeaveWithHostMigrationJob : public common::StepSequenceJob
{
public:
    enum
    {
        cStationNumMax = 32
    };

    LeaveWithHostMigrationJob();
    virtual ~LeaveWithHostMigrationJob();

    bool Startup(common::CallContext* context);
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
    virtual int DecideNextHost() = 0;
    virtual void CleanupStatus();

    void setStep(unsigned int (LeaveWithHostMigrationJob::*function)(), const char* name)
    {
        union
        {
            unsigned int (LeaveWithHostMigrationJob::*derived)();
            unsigned int (common::StepSequenceJob::*base)();
        } step;
        step.derived = function;
        m_currentStep.m_function = step.base;
        m_currentStep.m_name = name;
    }

    common::CallContext* m_callerContext;
    common::Time m_deadline;
    int m_timeoutMsec;
    bool m_started;
    StationIndex m_newHost;
    bool m_stationResponses[cStationNumMax];
    bool m_migrationStarted;
};

}}}

#endif
