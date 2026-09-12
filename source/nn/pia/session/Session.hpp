#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace common { class Job; }}}

namespace nn { namespace pia { namespace session {

class IMatchmakeSession;
class JoinSessionJob;
class JoinSessionSetting;
class JointSessionJob;
class MeshLayerController;
class SessionStatusCheckJob;
class UpdateSessionSetting;
class UpdateSessionSettingJob;

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

    void ResetInternalAsyncState();
    Result JoinSessionAsync(const JoinSessionSetting* setting);
    Result UpdateSessionSettingAsync(const UpdateSessionSetting* setting);
    Result UpdateSessionSettingAsyncCore(unsigned int sessionId,
                                         IMatchmakeSession* matchmakeSession,
                                         const UpdateSessionSetting* setting,
                                         bool isJointSession);

    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    JointSessionJob* m_jointSessionJob;
    JoinSessionJob* m_joinSessionJob;
    UpdateSessionSettingJob* m_updateSessionSettingJob;
    common::CallContext m_callContext;
    bool m_joinSessionJobRunning;
    unsigned int m_asyncOperationState;
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
