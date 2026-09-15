#include "nn/pia/lan/LanSessionStartupSetting.hpp"

#include <cstring>

namespace nn { namespace pia { namespace lan {

LanSessionStartupSetting::LanSessionStartupSetting()
    : m_field00(4000),
      m_field04(400),
      m_field08(0),
      m_field90(1),
      m_cryptoSetting()
{
    std::memset(m_stationPlayerInfo, 0, sizeof(m_stationPlayerInfo));
    std::memset(m_field91, 0, sizeof(m_field91));
}

}}}
