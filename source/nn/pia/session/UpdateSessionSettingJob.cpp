#include "nn/pia/session/UpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace session {

UpdateSessionSettingJob::UpdateSessionSettingJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_sessionId(0)
{
}

UpdateSessionSettingJob::~UpdateSessionSettingJob()
{
}

void UpdateSessionSettingJob::Trace(unsigned long long value) const
{
}

}}}
