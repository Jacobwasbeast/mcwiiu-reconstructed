#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace common { class Job; }}}

namespace nn { namespace pia { namespace session {

class IMatchmakeSession;
class JointSessionJob;
class MeshLayerController;
class SessionStatusCheckJob;

class Session
{
public:
    typedef unsigned int ProcessType;

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

    typedef bool (*JoiningApprovalCallback)(
        const transport::Station::IdentificationToken* identificationToken);

    Result Startup(const StartupSetting& setting);
    void SetSyncClockPulseInterval(int interval);
    bool IsHost() const;
    bool IsJointSessionHost() const;
    transport::Station::IdentificationToken* GetMatchmakeAnalysisToken();
    static bool SessionJoiningApprovalCallback(
        const transport::Station::IdentificationToken* identificationToken);

    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    JointSessionJob* m_jointSessionJob;
    JoiningApprovalCallback m_joiningApprovalCallback;
    unsigned int m_startupState;
    unsigned int m_matchmakeStatus;
    unsigned int m_disconnectStatus;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    unsigned long long m_jointSessionHostStationId;
    unsigned int m_sessionDisconnectReason;
    unsigned char m_currentSessionIndex;
    IMatchmakeSession* m_matchmakeSession[2];
    unsigned int m_sessionId[2];
    common::ListBase m_validStationIdList;
    common::ListBase m_validStationIdFreeList;
    int m_stationIdNodeOffset;
    transport::Station::IdentificationToken m_matchmakeAnalysisToken;
    unsigned short m_playerNum;
    unsigned short m_participantNum;
};

extern Session* g_piaSession;
extern unsigned char g_piaMatchmakeAnalysisEnabled;

}}}

#endif
