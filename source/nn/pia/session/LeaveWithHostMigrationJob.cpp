#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"

namespace nn { namespace pia { namespace session {

LeaveWithHostMigrationJob::LeaveWithHostMigrationJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_deadlineHigh(0),
      m_deadlineLow(0),
      m_timeoutMsec(5000),
      m_started(false),
      m_newHost(0xfd),
      m_stationResponses(),
      m_multiMigration(false)
{
    for (unsigned int index = 0; index < sizeof(m_stationResponses); ++index)
        m_stationResponses[index] = 0;
}

LeaveWithHostMigrationJob::~LeaveWithHostMigrationJob()
{
    Cleanup();
}

void LeaveWithHostMigrationJob::ReceiveMigrationResponse(StationIndex station)
{
    const unsigned int index = static_cast<unsigned int>(station);
    if (index < sizeof(m_stationResponses))
        m_stationResponses[index] = 0;
}

void LeaveWithHostMigrationJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    CleanupStatus();
}

void LeaveWithHostMigrationJob::Trace(unsigned long long value) const
{
}

void LeaveWithHostMigrationJob::CleanupStatus()
{
    m_started = false;
    m_deadlineHigh = 0;
    m_deadlineLow = 0;
    m_newHost = 0xfd;
    m_multiMigration = false;
    for (unsigned int index = 0; index < sizeof(m_stationResponses); ++index)
        m_stationResponses[index] = 0;
}

}}}
