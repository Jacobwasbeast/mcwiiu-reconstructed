#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHUPDATESESSIONSETTINGJOB_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class IMatchmakeSession; class UpdateSessionSetting; }}}

namespace nn { namespace pia { namespace lan {

class LanMatchUpdateSessionSettingJob
{
public:
    LanMatchUpdateSessionSettingJob();
    ~LanMatchUpdateSessionSettingJob();
    Result StartupImpl(uint32_t gatheringId, session::IMatchmakeSession* matchmakeSession, const session::UpdateSessionSetting* setting, bool notify);
    uint32_t UpdateSessionSetting();
    uint32_t WaitUpdateSessionSetting();
    void Cleanup();
    void Trace(uint64_t value) const;

private:
    unsigned char m_storage[1];
};

}}}

#endif
