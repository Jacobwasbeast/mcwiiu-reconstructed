#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_SESSION_STARTUP_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_SESSION_STARTUP_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace inet {

class NexSessionStartupSetting : public session::Session::StartupSetting
{
public:
    NexSessionStartupSetting();

    unsigned int m_payloadSize;
    transport::Station::PlayerInfo m_playerInfo[1];
    bool m_playersCountedAsParticipants;
    void* m_field94;
    bool m_hostMigrationEnabled;
    bool m_field99;
    int m_bandwidthCheckMeasurementTime;
    int m_bandwidthCheckMeasurementCount;
    unsigned int m_bandwidthCheckIpPacketSize;
    bool m_recentPlayRecordEnabled;
};

}}}

#endif
