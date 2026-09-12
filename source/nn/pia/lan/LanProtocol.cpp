#include "nn/pia/lan/LanProtocol.hpp"

namespace nn { namespace pia { namespace lan {

Result LanProtocol::Bind(const LanFacade::LanNetworkSetting& setting)
{
    if (setting.m_field00 == 0)
        return 0x10c07;

    m_field18 = setting.m_field00;
    m_field1c = (setting.m_field00 & setting.m_field04) | ~setting.m_field04;
    m_field24 = setting.m_field08;
    return 0;
}

}}}
