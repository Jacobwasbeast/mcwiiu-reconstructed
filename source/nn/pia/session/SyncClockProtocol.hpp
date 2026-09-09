#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_PROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SYNC_CLOCK_PROTOCOL_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace session {

class SyncClockProtocol : public transport::Protocol
{
public:
    enum { cProtocolType = 0x1c };

    SyncClockProtocol();
    virtual ~SyncClockProtocol();
    virtual unsigned short GetProtocolType() const;

    int m_pulseInterval;
};

}}}

#endif
