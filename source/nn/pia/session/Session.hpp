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

    enum EventType : unsigned int
    {
        cEventTypeJoin = 0,
        cEventTypeLeave = 1
    };

    enum SessionOpenStatus
    {
        cSessionOpenStatusClosed = 0,
        cSessionOpenStatusOpen = 1
    };

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

    class Setting
    {
    public:
        unsigned int m_field00;
        unsigned int m_field04;
        unsigned short m_field08;
        unsigned short m_field0a;
        unsigned int m_field0c;
        unsigned int m_field10;
    };
    static_assert(sizeof(Setting) == 20, "Session::Setting ABI");

    static Result CreateInstance(const Setting& setting);

    typedef bool (*JoiningApprovalCallback)(
        const transport::Station::IdentificationToken* identificationToken);

    Result Startup(const StartupSetting& setting);
    void SetSyncClockPulseInterval(int interval);
    bool IsHost() const;
    bool IsJointSessionHost() const;
    unsigned int GetStatus() const;
    unsigned int GetJoinMeshState() const;
    unsigned int GetSessionOpenStatus() const;
    void SetCurrentSessionOpenStatus(SessionOpenStatus status);
    void SetBufferSessionOpenStatus(SessionOpenStatus status);
    bool UpdateStationIdToBecomeClient();
    void ChangeStatusToDisconnectedNetwork();
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

    unsigned char m_prefix[0x98];
    common::CallContext m_callContext;
    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    JointSessionJob* m_jointSessionJob;
    JoinSessionJob* m_joinSessionJob;
    UpdateSessionSettingJob* m_updateSessionSettingJob;
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
    unsigned int m_sessionOpenStatus[2];
    common::ListBase m_validStationIdList;
    common::ListBase m_validStationIdFreeList;
    int m_stationIdNodeOffset;
    transport::Station::IdentificationToken m_matchmakeAnalysisToken;
    unsigned short m_playerNum;
    unsigned short m_participantNum;
};

static_assert(offsetof(Session, m_callContext) == 0x98, "Session::CallContext ABI");
static_assert(sizeof(Session) == 0x150, "Session ABI");

extern Session* g_piaSession;
extern unsigned char g_piaMatchmakeAnalysisEnabled;

}}}

#endif
