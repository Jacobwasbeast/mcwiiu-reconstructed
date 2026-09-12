#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONINFO_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/lan/LanStationInfo.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace lan {

class LanSessionInfo
{
public:
    LanSessionInfo();
    virtual ~LanSessionInfo();

    void Copy(const LanSessionInfo& other);

    unsigned int m_field04;
    unsigned int m_field08;
    unsigned short m_field0c;
    unsigned short m_field0e;
    unsigned short m_field10;
    unsigned char m_field12;
    unsigned int m_attribute[6];
    unsigned char m_field2c;
    unsigned char m_field2d;
    unsigned short m_field2e;
    unsigned char m_reserved30[32];
    unsigned char m_applicationData[384];
    unsigned int m_applicationDataSize;
    transport::StationConnectionInfo m_stationConnectionInfo;
    LanStationInfo m_stationInfo[16];
    unsigned int m_stationInfoCount;
};

}}}

#endif
