#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSION_HPP

#include <cstddef>

#include "nn/pia/Common.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn { namespace pia { namespace common { class Job; class StepSequenceJob; class String; }}}

namespace nn { namespace pia { namespace session {

class BrowseMatchmakeJob;
class CloseParticipationJob;
class CreateSessionJob;
class DestroySessionJob;
class ICommunityInfoList;
class IMatchmakeSession;
class ISessionInfoList;
class JoinSessionJob;
class JoinSessionSetting;
class JointSessionJob;
class LeaveSessionJob;
class MeshEventListener;
class MeshLayerController;
class OpenParticipationJob;
class RandomMatchmakeJob;
class SessionProtocol;
class SessionStatusCheckJob;
class StationIdStatusTable;
class UpdateSessionSetting;
class UpdateSessionSettingJob;

class Session
{
public:
    enum MatchmakeStatus
    {
        cMatchmakeStatusNone = 0,
        cMatchmakeStatusSearching = 1,
        cMatchmakeStatusMatched = 2
    };

    enum ConnectionStatus
    {
        cConnectionStatusNone = 0,
        cConnectionStatusConnecting = 1,
        cConnectionStatusConnected = 2
    };

    typedef unsigned int ProcessType;

    enum EventType
    {
        cEventTypeJoin = 0,
        cEventTypeLeave = 1
    };

    enum SessionOpenStatus
    {
        cSessionOpenStatusClosed = 0,
        cSessionOpenStatusOpen = 1
    };

    enum AsyncProcessingName
    {
        cAsyncProcessingNameNone = 0,
        cAsyncProcessingNameJoinRandomSession = 1,
        cAsyncProcessingNameBrowseSession = 2,
        cAsyncProcessingNameCreateSession = 3,
        cAsyncProcessingNameJoinSession = 4,
        cAsyncProcessingNameLeaveSession = 5,
        cAsyncProcessingNameUpdateSessionSetting = 0x11
    };

    typedef void (*SessionEventCallback)(EventType eventType,
                                         unsigned long long stationId,
                                         void* param);

    struct StationIdEntry
    {
        common::ListNode m_node;
        unsigned long long m_stationId;
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

    static Result CreateInstance(const Setting& setting);

    typedef bool (*JoiningApprovalCallback)(
        const transport::Station::IdentificationToken* identificationToken);

    Result Startup(const StartupSetting& setting);
    void SetSyncClockPulseInterval(int interval);
    bool IsHost() const;
    bool IsValidStationId(unsigned long long stationId) const;
    bool IsJointSessionHost() const;
    unsigned int GetStatus() const;
    unsigned int GetJoinMeshState() const;
    unsigned int GetSessionOpenStatus() const;
    void SetCurrentSessionOpenStatus(SessionOpenStatus status);
    void SetBufferSessionOpenStatus(SessionOpenStatus status);
    bool UpdateStationIdToBecomeClient();
    bool UpdateStationIdToBecomeHost();
    void ChangeStatusToDisconnectedSession();
    void CheckStatusAtChangeMeshState();
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

    bool m_selectionPriorityEnabled;
    MeshLayerController* m_meshLayerController;
    SessionStatusCheckJob* m_sessionStatusCheckJob;
    CreateSessionJob* m_createSessionJob;
    RandomMatchmakeJob* m_randomMatchmakeJob;
    BrowseMatchmakeJob* m_browseMatchmakeJob;
    JoinSessionJob* m_joinSessionJob;
    LeaveSessionJob* m_leaveSessionJob;
    DestroySessionJob* m_destroySessionJob;
    OpenParticipationJob* m_openParticipationJob;
    CloseParticipationJob* m_closeParticipationJob;
    common::StepSequenceJob* m_generateMatchmakeSystemPasswordJob;
    common::StepSequenceJob* m_clearMatchmakeSystemPasswordJob;
    common::String* m_matchmakeSystemPassword;
    JointSessionJob* m_jointSessionJob;
    UpdateSessionSettingJob* m_updateSessionSettingJob;
    unsigned int m_field40;
    common::StepSequenceJob* m_communityManagementJob;
    unsigned int m_field48;
    unsigned int m_field4c;
    SessionEventCallback m_sessionEventCallback;
    void* m_sessionEventCallbackParam;
    JoiningApprovalCallback m_joiningApprovalCallback;
    StationIdStatusTable* m_stationIdStatusTable;
    unsigned short m_protocolId;
    SessionProtocol* m_sessionProtocol;
    unsigned int m_startupState;
    unsigned int m_matchmakeStatus;
    unsigned int m_disconnectStatus;
    unsigned int m_field74;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    unsigned long long m_jointSessionHostStationId;
    unsigned long long m_field90;
    common::CallContext m_callContext;
    AsyncProcessingName m_asyncProcessingName;
    MeshEventListener* m_meshEventListener;
    unsigned int m_waitingStationIndex;
    unsigned char m_fieldb8;
    unsigned int m_sessionDisconnectReason;
    unsigned char m_fieldc0;
    bool m_postponeNotice;
    unsigned short m_fieldc2;
    unsigned short m_fieldc4;
    unsigned char m_currentSessionIndex;
    IMatchmakeSession* m_matchmakeSession[2];
    ISessionInfoList* m_sessionInfoList;
    unsigned int m_sessionId[2];
    ICommunityInfoList* m_communityInfoList;
    unsigned int m_sessionOpenStatus[2];
    common::ListBase m_validStationIdList;
    common::ListBase m_validStationIdFreeList;
    int m_stationIdNodeOffset;
    StationIdEntry* m_stationIdEntries;
    unsigned int m_stationIdEntryNum;
    StationIdEntry* m_stationIdEntryBuffer;
    unsigned int m_field110;
    bool m_hostMigrationOccurred;
    transport::Station::IdentificationToken m_matchmakeAnalysisToken;
    unsigned short m_playerNum;
    unsigned short m_participantNum;
    unsigned int m_field13c;
    unsigned char m_field140;
    unsigned int m_field144;
    unsigned char m_field148;
};


extern Session* g_piaSession;
extern unsigned char g_piaMatchmakeAnalysisEnabled;

}}}

#endif
