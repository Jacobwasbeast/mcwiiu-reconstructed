#include "nn/pia/lan/LanMatchUpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchUpdateSessionSettingJob::LanMatchUpdateSessionSettingJob()
{
}

LanMatchUpdateSessionSettingJob::~LanMatchUpdateSessionSettingJob()
{
}

Result LanMatchUpdateSessionSettingJob::StartupImpl(uint32_t gatheringId, session::IMatchmakeSession* matchmakeSession, const session::UpdateSessionSetting* setting, bool notify)
{
    return 0;
}

uint32_t LanMatchUpdateSessionSettingJob::UpdateSessionSetting()
{
    return 0;
}

uint32_t LanMatchUpdateSessionSettingJob::WaitUpdateSessionSetting()
{
    return 0;
}

void LanMatchUpdateSessionSettingJob::Cleanup()
{
}

void LanMatchUpdateSessionSettingJob::Trace(uint64_t value) const
{
}

}}}
