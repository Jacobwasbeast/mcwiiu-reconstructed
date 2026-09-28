#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace inet {

class NexMatchUpdateSessionSettingJob
{
public:
    NexMatchUpdateSessionSettingJob();
    ~NexMatchUpdateSessionSettingJob();
    uint64_t StartupImpl(uint32_t a1, int32_t a2, int32_t a3, int32_t a4, uint8_t a5);
    uint32_t UpdateSessionSetting();
    uint32_t WaitUpdateSessionSetting();
    uint32_t ModifyAttribute();
    uint32_t WaitModifyAttribute();
    uint32_t UpdateAttributes();
    uint32_t WaitUpdateAttributes();
    uint32_t UpdateApplicationData();
    uint32_t WaitUpdateApplicationData();
    void Cleanup();
    void Trace(uint64_t a1);

private:
    uint64_t state_;
};

}}}

#endif
