#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_SESSION_STARTUP_SETTING_HPP

#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace lan {

class LanSessionStartupSetting : public session::Session::StartupSetting
{
public:
    LanSessionStartupSetting();

    unsigned int m_payloadWord;
    transport::Station::PlayerInfo m_playerInfo[1];
    bool m_playersCountedAsParticipants;
    common::CryptoSetting m_cryptoSetting;
};

}}}

#endif
