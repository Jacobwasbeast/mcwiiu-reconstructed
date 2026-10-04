#include "nn/pia/lan/LanSessionStartupSetting.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cDefaultField00 = 4000,
    cDefaultField04 = 400,
    cCryptoKeySize = 0x10
};
}

LanSessionStartupSetting::LanSessionStartupSetting()
    : session::Session::StartupSetting(), m_playerInfo(), m_playersCountedAsParticipants(true), m_cryptoSetting()
{
    m_cryptoSetting.m_mode = 1;
    m_field04 = cDefaultField04;
    m_field00 = cDefaultField00;
    for (unsigned int index = 0; index < cCryptoKeySize; ++index)
        m_cryptoSetting.m_key[index] = 0;
}

}}}
