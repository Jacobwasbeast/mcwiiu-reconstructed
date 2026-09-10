#include "nn/nex/PeriodicJob.hpp"

namespace nn
{
namespace nex
{

PeriodicJob::PeriodicJob(JobType jobType, const DebugString& rDebugString)
    : Job(jobType, rDebugString)
    , m_iPeriod(0)
{
    SetToWaiting(m_waitContext);
}

PeriodicJob::~PeriodicJob()
{
}

void PeriodicJob::SetDefaultPostExecutionState()
{
    if (m_iPeriod > 0)
    {
        SetToWaiting(m_iPeriod);
        return;
    }

    SetToComplete();
}

std::uint32_t PeriodicJob::SkipWaitDelayAtTermination()
{
    return m_iPeriod == 0;
}

}
}
