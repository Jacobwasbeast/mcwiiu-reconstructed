#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP

#include "nn/nex/qVector.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/inet/SessionType.hpp"

namespace nn { namespace nex {

enum MatchmakeSystemType
{
    cMatchmakeSystemType_1 = 1,
    cMatchmakeSystemType_2 = 2
};

class MatchmakeSession;
class PersistentGathering;

}}

namespace nn { namespace pia { namespace session {

class CreateSessionSetting;
class JoinSessionSetting;
class JoinRandomSessionSetting;
class UpdateSessionSetting;

}}}

namespace nn { namespace pia { namespace transport {

class StationConnectionInfo;

}}}

namespace nn { namespace pia { namespace inet {

class NexCommunityInfo;
class NexSessionInfo;
class NexCreateCommunitySetting;
class NexJoinCommunitySetting;
class NexUpdateCommunitySetting;
class NexSessionSearchCriteriaOwner;
class NexSessionSearchCriteriaParticipant;
class NexSessionSearchCriteriaSessionId;

class NexMatchmakeSession
{
public:
    NexMatchmakeSession();
    virtual ~NexMatchmakeSession();
    unsigned long long BrowseMatchmakeSession(nn::pia::common::CallContext* callContext);
    void Cleanup();
    unsigned long long ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId);
    static unsigned int ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType);
    unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long code);
    static unsigned int ConvertSelectionMethodToMatchmakeSelectionMethod(SelectionMethod method);
    static unsigned int ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType);
    void CopyFoundPersistentGatheringToBrowsedCommunityInfoList(const nn::nex::PersistentGathering* gathering, NexCommunityInfo* communityInfo);
    void CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session, NexSessionInfo* sessionInfo);
    unsigned long long CreateCommunity(nn::pia::common::CallContext* callContext);
    unsigned long long CreateMatchmakeSession(nn::pia::common::CallContext* callContext);
    unsigned long long CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId);
    unsigned long long Finalize();
    unsigned long long FindCommunityByGatheringId(nn::pia::common::CallContext* callContext);
    unsigned long long FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId, unsigned int offset, unsigned int size);
    unsigned long long FindCommunityByParticipant(nn::pia::common::CallContext* callContext, unsigned long long participantId, unsigned int offset, unsigned int size);
    unsigned long long FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isOfficial, unsigned int offset, unsigned int size);
    unsigned long long FindSessionByOwner(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaOwner& criteria);
    unsigned long long FindSessionByParticipant(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaParticipant& criteria);
    unsigned long long FindSessionBySessionId(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaSessionId& criteria);
    unsigned long long GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned int GetBrowsedCommunityResult();
    unsigned int GetBrowseMatchmakeSessionResult();
    unsigned int GetRandomMatchmakeScore() const;
    int GetSessionInfo();
    unsigned long long GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId);
    unsigned long long Initialize();
    unsigned char IsMatchmakeHost() const;
    unsigned char IsProcessingJoinMatchmakeSession() const;
    unsigned char IsRandomMatchmakeScoreAvailable() const;
    unsigned int IsUpdatableSelectionPriority(unsigned char priority);
    unsigned long long JoinCommunity(nn::pia::common::CallContext* callContext);
    unsigned long long JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int teamIndex);
    unsigned long long LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* candidateIds, unsigned short candidateCount);
    unsigned long long ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int index, unsigned int value);
    unsigned long long OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long RandomMatchmakeSession(nn::pia::common::CallContext* callContext);
    unsigned long long RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId);
    unsigned long long ReflectCommunityIdList(const unsigned int* idList, unsigned int count);
    unsigned long long ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting);
    void ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting, bool isTeam);
    unsigned long long ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting);
    unsigned long long ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting, bool isTeam);
    unsigned long long ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting);
    unsigned long long ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting, bool isTeam);
    unsigned long long ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting);
    unsigned long long ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting, bool isTeam);
    void RegisterFindCommunityResult();
    void RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType);
    unsigned long long RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria, unsigned char flags);
    unsigned long long RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    void SetMatchmakeCurrentParticipantNum(unsigned short participantNum);
    void SetMatchmakeOwnerUniqueId(unsigned long long ownerId);
    unsigned long long SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize);
    unsigned long long SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key);
    void SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId);
    unsigned long long UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    void UnregisterSearchCriteria();
    unsigned long long UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId, const unsigned char* buffer, unsigned int size);
    unsigned long long UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int* attributes);
    unsigned long long UpdateCommunity(nn::pia::common::CallContext* callContext);
    unsigned long long UpdateMatchmakeSessionHost(unsigned int stationId);
    unsigned long long UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority);
    unsigned long long UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned int WaitBrowseMatchmakeSession();
    unsigned int WaitClearSystemPassword();
    unsigned int WaitCloseParticipate();
    unsigned int WaitCreateCommunity();
    unsigned int WaitCreateMatchmakeSession(unsigned int* gatheringId);
    unsigned int WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId);
    unsigned int WaitFindCommunityByGatheringId();
    unsigned int WaitFindCommunityByOwner();
    unsigned int WaitFindCommunityByParticipant();
    unsigned int WaitFindOfficialCommunity();
    unsigned int WaitFindSessionByOwner();
    unsigned int WaitFindSessionByParticipant();
    unsigned int WaitFindSessionBySessionId();
    unsigned int WaitGenerateSystemPassword();
    unsigned int WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info);
    unsigned int WaitJoinCommunity();
    unsigned int WaitJoinMatchmakeSession();
    unsigned int WaitJoinMatchmakeSessionWithTeam();
    unsigned int WaitLeaveMatchmakeSession();
    unsigned int WaitLeaveMatchmakeSessionCompanion();
    unsigned int WaitMigrateMatchmakeSessionOwner();
    unsigned int WaitModifyAttribute();
    unsigned int WaitOpenParticipate();
    unsigned int WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found);
    unsigned int WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found);
    unsigned int WaitRequestSessionInfo();
    unsigned int WaitRequestSessionInfoWithoutPermission();
    unsigned int WaitUnregisterMatchmakeSession();
    unsigned int WaitUpdateApplicationBuffer();
    unsigned int WaitUpdateAttributes();
    unsigned int WaitUpdateCommunity();
    unsigned int WaitUpdateSessionSetting();

private:
    unsigned int m_state;
};

}}}

#endif
