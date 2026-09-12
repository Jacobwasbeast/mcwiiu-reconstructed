#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANFACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANFACADE_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace lan {

class LanProtocol;

class LanFacade
{
public:
    class LanNetworkSetting
    {
    public:
        LanNetworkSetting()
            : m_field00(0), m_field04(0), m_field08(0)
        {
        }

        unsigned int m_field00;
        unsigned int m_field04;
        unsigned char m_field08;
    };

    LanFacade();
    ~LanFacade();

    Result Initialize();
    Result Bind(const LanNetworkSetting& setting);

    static Result CreateInstance();
    static LanFacade* GetInstance();

    LanProtocol* m_lanProtocol;
    LanNetworkSetting m_networkSetting;
};

extern LanFacade* g_lanFacade;

}}}

#endif
