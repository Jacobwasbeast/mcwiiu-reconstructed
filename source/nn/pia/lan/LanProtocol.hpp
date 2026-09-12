#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANPROTOCOL_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/lan/LanFacade.hpp"

namespace nn { namespace pia { namespace lan {

class LanProtocol : public transport::Protocol
{
public:
    Result Bind(const LanFacade::LanNetworkSetting& setting);

private:
    unsigned int m_field18;
    unsigned int m_field1c;
    unsigned char m_field24;
};

}}}

#endif
