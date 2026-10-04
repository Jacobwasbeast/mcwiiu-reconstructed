#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class IMatchmakeSession; class SessionSearchCriteria; }}}

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchBrowseMatchmakeJob
{
public:
    LanMatchBrowseMatchmakeJob();
    ~LanMatchBrowseMatchmakeJob();
    void Cleanup();
    uint32_t BrowseMatchmake();
    uint32_t WaitBrowseMatchmake();
    uint32_t WaitRequestSessionInfo();
    void Trace(uint64_t a1) const;
    Result StartupImpl(session::IMatchmakeSession* matchmake, const session::SessionSearchCriteria* criteria);
    Result StartupImpl(session::IMatchmakeSession* matchmake, unsigned int a2);

private:
    unsigned char m_storage[96];
};

}
}
}

#endif
