#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP

#include "nn/pia/common/CryptoSetting.hpp"

namespace nn { namespace pia { namespace lan {

class LanSessionStartupSetting
{
public:
    LanSessionStartupSetting();

private:
    unsigned int m_field00;
    unsigned int m_field04;
    unsigned int m_field08;
    unsigned char m_stationPlayerInfo[128];
    unsigned char m_field90;
    unsigned char m_field91[3];
    common::CryptoSetting m_cryptoSetting;
};

}}}

#endif
