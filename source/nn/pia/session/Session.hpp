#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace common { class Job; }}}

namespace nn { namespace pia { namespace session {

class MeshLayerController;
class SessionStatusCheckJob;

class Session
{
public:
    class StartupSetting
    {
    public:
        StartupSetting()
            : m_field00(10000), m_field04(1000), m_identificationToken(0)
        {
        }

        unsigned int m_field00;
        unsigned int m_field04;
        const transport::Station::IdentificationToken* m_identificationToken;
    };

    Result Startup(const StartupSetting& setting);
    void SetSyncClockPulseInterval(int interval);

    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    common::Job* m_jointSessionJob;
    unsigned int m_startupState;
    unsigned int m_disconnectStatus;
    unsigned int m_sessionDisconnectReason;
    common::ListBase m_validStationIdList;
    common::ListBase m_validStationIdFreeList;
    int m_stationIdNodeOffset;
    unsigned short m_playerNum;
    unsigned short m_participantNum;
};

extern Session* g_piaSession;

}}}

#endif
