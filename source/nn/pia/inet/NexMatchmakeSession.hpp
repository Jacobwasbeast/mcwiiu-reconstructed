#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP

#include <list>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/AutoMatchmakeParam.hpp"
#include "nn/nex/CreateMatchmakeSessionParam.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/JoinMatchmakeSessionParam.hpp"
#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/PersistentGathering.hpp"
#include "nn/nex/PlayingSession.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/UpdateMatchmakeSessionParam.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qVector.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/inet/NexSessionInfo.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/CommonMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"

namespace nn { namespace nex {

class MatchmakeExtensionClient;

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
class NexCreateCommunitySetting;
class NexJoinCommunitySetting;
class NexUpdateCommunitySetting;
class NexSessionSearchCriteriaOwner;
class NexSessionSearchCriteriaParticipant;
class NexSessionSearchCriteriaSessionId;

class NexMatchmakeSession : public session::CommonMatchmakeSession
{
public:
    NexMatchmakeSession();
    virtual ~NexMatchmakeSession();
    virtual void Cleanup();
    virtual Result RandomMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found);
    virtual Result BrowseMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitBrowseMatchmakeSession();
    virtual session::ISessionInfoList* GetBrowseMatchmakeSessionResult();
    virtual Result CreateMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitCreateMatchmakeSession(unsigned int* gatheringId);
    virtual Result JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitJoinMatchmakeSession();
    virtual Result LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitLeaveMatchmakeSession();
    virtual Result UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUnregisterMatchmakeSession();
    virtual Result OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitOpenParticipate();
    virtual Result CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitCloseParticipate();
    virtual Result GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId);
    virtual bool WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info);
    virtual Result RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitRequestSessionInfo();
    virtual unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId);
    virtual unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long code);
    virtual unsigned int GetBrowsedCommunityResult();
    virtual Result UpdateMatchmakeSessionHost(unsigned int stationId);
    virtual void SetMatchmakeOwnerUniqueId(unsigned long long ownerId);
    virtual Result UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority);
    virtual unsigned int IsUpdatableSelectionPriority(unsigned char priority);
    virtual Result RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId);
    virtual bool WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found);
    virtual Result FindSessionByOwner(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaOwner& criteria);
    virtual bool WaitFindSessionByOwner();
    virtual Result FindSessionByParticipant(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaParticipant& criteria);
    virtual bool WaitFindSessionByParticipant();
    virtual Result FindSessionBySessionId(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaSessionId& criteria);
    virtual bool WaitFindSessionBySessionId();
    virtual Result CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId);
    virtual bool WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId);
    virtual Result JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int teamIndex);
    virtual bool WaitJoinMatchmakeSessionWithTeam();
    virtual Result UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUpdateSessionSetting();
    virtual void SetMatchmakeCurrentParticipantNum(unsigned short participantNum);

    unsigned long long ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    static SessionType ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType);
    static unsigned int ConvertSelectionMethodToMatchmakeSelectionMethod(SelectionMethod method);
    static nn::nex::MatchmakeSystemType ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType);
    void CopyFoundPersistentGatheringToBrowsedCommunityInfoList(const nn::nex::PersistentGathering* gathering, NexCommunityInfo* communityInfo);
    void CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session, NexSessionInfo* sessionInfo);
    unsigned long long CreateCommunity(nn::pia::common::CallContext* callContext);
    unsigned long long Finalize();
    unsigned long long FindCommunityByGatheringId(nn::pia::common::CallContext* callContext);
    unsigned long long FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId, unsigned int offset, unsigned int size);
    unsigned long long FindCommunityByParticipant(nn::pia::common::CallContext* callContext, unsigned long long participantId, unsigned int offset, unsigned int size);
    unsigned long long FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isOfficial, unsigned int offset, unsigned int size);
    unsigned long long GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned int GetRandomMatchmakeScore() const;
    unsigned long long Initialize();
    unsigned char IsMatchmakeHost() const;
    unsigned char IsProcessingJoinMatchmakeSession() const;
    unsigned char IsRandomMatchmakeScoreAvailable() const;
    unsigned long long JoinCommunity(nn::pia::common::CallContext* callContext);
    unsigned long long MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* candidateIds, unsigned short candidateCount);
    unsigned long long ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int index, unsigned int value);
    unsigned long long ReflectCommunityIdList(const unsigned int* idList, unsigned int count);
    unsigned long long ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting);
    void ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting, bool isTeam);
    unsigned long long ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting);
    unsigned long long ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting, bool isTeam);
    unsigned long long ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting);
    Result ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting, bool hostMigrationEnabled);
    unsigned long long ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting);
    Result ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting, bool isTeam);
    void RegisterFindCommunityResult();
    void RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType);
    unsigned long long RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria, unsigned char flags);
    unsigned long long RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned long long SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize);
    unsigned long long SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key);
    void SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId);
    void UnregisterSearchCriteria();
    unsigned long long UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId, const unsigned char* buffer, unsigned int size);
    unsigned long long UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int* attributes);
    unsigned long long UpdateCommunity(nn::pia::common::CallContext* callContext);
    unsigned int WaitClearSystemPassword();
    unsigned int WaitCreateCommunity();
    unsigned int WaitFindCommunityByGatheringId();
    unsigned int WaitFindCommunityByOwner();
    unsigned int WaitFindCommunityByParticipant();
    unsigned int WaitFindOfficialCommunity();
    unsigned int WaitGenerateSystemPassword();
    unsigned int WaitJoinCommunity();
    unsigned int WaitLeaveMatchmakeSessionCompanion();
    unsigned int WaitMigrateMatchmakeSessionOwner();
    unsigned int WaitModifyAttribute();
    unsigned int WaitRequestSessionInfoWithoutPermission();
    unsigned int WaitUpdateApplicationBuffer();
    unsigned int WaitUpdateAttributes();
    unsigned int WaitUpdateCommunity();

private:
    typedef std::list<nn::nex::MatchmakeSessionSearchCriteria, nn::nex::MemAllocator<nn::nex::MatchmakeSessionSearchCriteria> > SearchCriteriaList;
    typedef std::list<nn::nex::ResultRange, nn::nex::MemAllocator<nn::nex::ResultRange> > ResultRangeList;
    typedef std::list<unsigned int, nn::nex::MemAllocator<unsigned int> > IdList;

    void* m_field18;
    unsigned int m_field1c;
    nn::nex::MatchmakeExtensionClient* m_matchmakeExtensionClient;
    unsigned int m_field24;
    nn::nex::MatchmakeSession* m_requestSession;
    SearchCriteriaList* m_searchCriteriaList;
    ResultRangeList* m_resultRangeList;
    std::list<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String>, nn::nex::MemAllocator<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> > >* m_gatheringList;
    session::ISessionInfoList* m_browseMatchmakeSessionResult;
    nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> m_gatheringHolder;
    nn::nex::MatchmakeSession* m_foundSession;
    nn::nex::MatchmakeParam* m_matchmakeParam;
    nn::nex::AutoMatchmakeParam* m_autoMatchmakeParam;
    nn::nex::CreateMatchmakeSessionParam* m_createMatchmakeSessionParam;
    nn::nex::JoinMatchmakeSessionParam* m_joinMatchmakeSessionParam;
    nn::nex::UpdateMatchmakeSessionParam* m_updateMatchmakeSessionParam;
    nn::nex::PersistentGathering* m_persistentGathering;
    std::list<nn::nex::PersistentGathering, nn::nex::MemAllocator<nn::nex::PersistentGathering> >* m_persistentGatheringList;
    unsigned int m_browsedCommunityResult;
    IdList* m_idList68;
    nn::nex::ResultRange* m_resultRange;
    std::list<nn::nex::PlayingSession, nn::nex::MemAllocator<nn::nex::PlayingSession> >* m_playingSessionList;
    std::list<nn::nex::StationURL, nn::nex::MemAllocator<nn::nex::StationURL> >* m_stationUrlList;
    nn::nex::String* m_string78;
    IdList* m_idList7c;
    nn::nex::InetAddress* m_inetAddress;
    unsigned int m_field84;
    nn::nex::ProtocolCallContext m_protocolCallContext;
    nn::nex::ProtocolCallContext m_protocolCallContextB;
    nn::pia::common::CallContext* m_callContext;
    bool m_matchmakeHost;
    unsigned int m_field180;
    unsigned int m_field184;
    nn::pia::common::Time m_selectionPriorityUpdateTime;
    nn::pia::common::Time m_field190;
    NexSessionInfo m_sessionInfo;
    unsigned char m_sessionInfoReserved[16];
    unsigned int m_randomMatchmakeScore;
    bool m_randomMatchmakeScoreAvailable;
    bool m_processingJoinMatchmakeSession;
    unsigned int m_temporaryOwnerChangeGatheringId;
    unsigned long long m_temporaryOwnerChangeOwnerId;
    unsigned int m_field418[8];
};

}}}

#endif
