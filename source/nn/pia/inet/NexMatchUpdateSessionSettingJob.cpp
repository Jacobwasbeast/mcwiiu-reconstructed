#include "nn/pia/inet/NexMatchUpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace inet {

uint64_t NexMatchUpdateSessionSettingJob::StartupImpl(uint32_t a1, session::IMatchmakeSession* a2, const session::UpdateSessionSetting* a3, bool a4)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::UpdateSessionSetting()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::WaitUpdateSessionSetting()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::ModifyAttribute()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::WaitModifyAttribute()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::UpdateAttributes()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::WaitUpdateAttributes()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::UpdateApplicationData()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchUpdateSessionSettingJob::WaitUpdateApplicationData()
{
    return static_cast<uint32_t>(state_);
}

void NexMatchUpdateSessionSettingJob::Cleanup()
{
    state_ += 1;
}

void NexMatchUpdateSessionSettingJob::Trace(uint64_t a1) const
{
}

}}}
