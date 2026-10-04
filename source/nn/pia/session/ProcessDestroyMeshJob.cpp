#include "nn/pia/session/ProcessDestroyMeshJob.hpp"

namespace nn { namespace pia { namespace session {

ProcessDestroyMeshJob::ProcessDestroyMeshJob()
    : common::StepSequenceJob(), m_endTick(0), m_endTickHigh(0),
      m_timeout(1000), m_started(0)
{
}

ProcessDestroyMeshJob::~ProcessDestroyMeshJob()
{
}

void ProcessDestroyMeshJob::Cleanup()
{
    m_started = 0;
}

void ProcessDestroyMeshJob::Trace(unsigned long long) const
{
}

}}}
