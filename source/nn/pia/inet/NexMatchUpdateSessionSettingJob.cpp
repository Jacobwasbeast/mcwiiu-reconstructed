#include "nn/pia/inet/NexMatchUpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace inet {

uint64_t NexMatchUpdateSessionSettingJob::StartupImpl(uint32_t a1, int32_t a2, int32_t a3, int32_t a4, uint8_t a5)
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

void NexMatchUpdateSessionSettingJob::Trace(uint64_t a1)
{
    state_ += 1;
}

}}}
