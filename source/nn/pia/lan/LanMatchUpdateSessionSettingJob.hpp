#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace lan {

class LanMatchUpdateSessionSettingJob
{
public:
    LanMatchUpdateSessionSettingJob();
    ~LanMatchUpdateSessionSettingJob();
    uint64_t StartupImpl(uint32_t a1, int32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
    uint32_t UpdateSessionSetting();
    uint32_t WaitUpdateSessionSetting();
    void Cleanup();
    void Trace(uint64_t a1);

private:
    unsigned char m_storage[1];
};

}}}

#endif
