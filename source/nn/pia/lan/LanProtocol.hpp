#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/lan/LanFacade.hpp"

namespace nn { namespace pia { namespace lan {

class LanProtocol : public transport::Protocol
{
public:
    enum { cProtocolId = 0x4400 };
    Result Bind(const LanFacade::LanNetworkSetting& setting);

    unsigned int GetProtocolIdentifier() const { return m_field18; }
    bool IsRelayEnabled() const { return m_field24 != 0; }

private:
    unsigned int m_field18;
    unsigned int m_field1c;
    unsigned char m_field24;
};

}}}

#endif
