#include "nn/pia/session/JoinSessionJob.hpp"

namespace nn { namespace pia { namespace session {

JoinSessionJob::JoinSessionJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext()
{
}

JoinSessionJob::~JoinSessionJob()
{
}

void JoinSessionJob::Trace(unsigned long long value) const
{
}

}}}
