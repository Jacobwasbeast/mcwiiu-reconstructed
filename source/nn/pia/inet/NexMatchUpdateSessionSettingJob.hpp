#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHUPDATESESSIONSETTINGJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace session { class IMatchmakeSession; class UpdateSessionSetting; }}}

namespace nn { namespace pia { namespace inet {

class NexMatchUpdateSessionSettingJob
{
public:
    NexMatchUpdateSessionSettingJob();
    ~NexMatchUpdateSessionSettingJob();
    uint64_t StartupImpl(uint32_t a1, session::IMatchmakeSession* a2, const session::UpdateSessionSetting* a3, bool a4);
    uint32_t UpdateSessionSetting();
    uint32_t WaitUpdateSessionSetting();
    uint32_t ModifyAttribute();
    uint32_t WaitModifyAttribute();
    uint32_t UpdateAttributes();
    uint32_t WaitUpdateAttributes();
    uint32_t UpdateApplicationData();
    uint32_t WaitUpdateApplicationData();
    void Cleanup();
    void Trace(uint64_t a1) const;

private:
    uint64_t state_;
};

}}}

#endif
