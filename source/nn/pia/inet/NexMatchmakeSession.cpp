#include "nn/pia/inet/NexMatchmakeSession.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchmakeSession::NexMatchmakeSession() : m_state(0)
{}

NexMatchmakeSession::~NexMatchmakeSession()
{}

unsigned long long NexMatchmakeSession::BrowseMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
void NexMatchmakeSession::Cleanup()
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId)
{
    return m_state;

}
unsigned int NexMatchmakeSession::ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType)
{
    if (systemType == nn::nex::cMatchmakeSystemType_2) {
        return 1;
    }
    return 0;

}
unsigned int NexMatchmakeSession::ConvertPersistentGatheringCodeToGatheringId(unsigned long long code)
{
    return m_state;

}
unsigned int NexMatchmakeSession::ConvertSelectionMethodToMatchmakeSelectionMethod(SelectionMethod method)
{
    if (method == cSelectionMethod_1) {
        return 4;
    }
    if (method == cSelectionMethod_2) {
        return 5;
    }
    return 0;

}
unsigned int NexMatchmakeSession::ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType)
{
    if (sessionType == cSessionType_Normal) {
        return 1;
    }
    if (sessionType == cSessionType_Ranked) {
        return 2;
    }
    return 0;

}
void NexMatchmakeSession::CopyFoundPersistentGatheringToBrowsedCommunityInfoList(const nn::nex::PersistentGathering* gathering, NexCommunityInfo* communityInfo)
{
    m_state = 0;

}
void NexMatchmakeSession::CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session, NexSessionInfo* sessionInfo)
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::CreateCommunity(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::CreateMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::Finalize()
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindCommunityByGatheringId(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId, unsigned int offset, unsigned int size)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindCommunityByParticipant(nn::pia::common::CallContext* callContext, unsigned long long participantId, unsigned int offset, unsigned int size)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isOfficial, unsigned int offset, unsigned int size)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindSessionByOwner(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaOwner& criteria)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindSessionByParticipant(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaParticipant& criteria)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::FindSessionBySessionId(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaSessionId& criteria)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned int NexMatchmakeSession::GetBrowsedCommunityResult()
{
    return m_state;

}
unsigned int NexMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return m_state;

}
unsigned int NexMatchmakeSession::GetRandomMatchmakeScore() const
{
    return m_state;

}
int NexMatchmakeSession::GetSessionInfo()
{
    return m_state;

}
unsigned long long NexMatchmakeSession::GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::Initialize()
{
    return m_state;

}
unsigned char NexMatchmakeSession::IsMatchmakeHost() const
{
    return m_state;

}
unsigned char NexMatchmakeSession::IsProcessingJoinMatchmakeSession() const
{
    return m_state;

}
unsigned char NexMatchmakeSession::IsRandomMatchmakeScoreAvailable() const
{
    return m_state;

}
unsigned int NexMatchmakeSession::IsUpdatableSelectionPriority(unsigned char priority)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::JoinCommunity(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int teamIndex)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* candidateIds, unsigned short candidateCount)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int index, unsigned int value)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::RandomMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectCommunityIdList(const unsigned int* idList, unsigned int count)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting)
{
    return m_state;

}
void NexMatchmakeSession::ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting, bool isTeam)
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting, bool isTeam)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting, bool isTeam)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting, bool isTeam)
{
    return m_state;

}
void NexMatchmakeSession::RegisterFindCommunityResult()
{
    m_state = 0;

}
void NexMatchmakeSession::RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType)
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria, unsigned char flags)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
void NexMatchmakeSession::SetMatchmakeCurrentParticipantNum(unsigned short participantNum)
{
    m_state = 0;

}
void NexMatchmakeSession::SetMatchmakeOwnerUniqueId(unsigned long long ownerId)
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key)
{
    return m_state;

}
void NexMatchmakeSession::SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId)
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
void NexMatchmakeSession::UnregisterSearchCriteria()
{
    m_state = 0;

}
unsigned long long NexMatchmakeSession::UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId, const unsigned char* buffer, unsigned int size)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int* attributes)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::UpdateCommunity(nn::pia::common::CallContext* callContext)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::UpdateMatchmakeSessionHost(unsigned int stationId)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority)
{
    return m_state;

}
unsigned long long NexMatchmakeSession::UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitBrowseMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitClearSystemPassword()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitCloseParticipate()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitCreateCommunity()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitCreateMatchmakeSession(unsigned int* gatheringId)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindCommunityByGatheringId()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindCommunityByOwner()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindCommunityByParticipant()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindOfficialCommunity()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindSessionByOwner()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindSessionByParticipant()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitFindSessionBySessionId()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitGenerateSystemPassword()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitJoinCommunity()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitJoinMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitJoinMatchmakeSessionWithTeam()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitLeaveMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitLeaveMatchmakeSessionCompanion()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitMigrateMatchmakeSessionOwner()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitModifyAttribute()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitOpenParticipate()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found)
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitRequestSessionInfo()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitRequestSessionInfoWithoutPermission()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitUpdateApplicationBuffer()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitUpdateAttributes()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitUpdateCommunity()
{
    return m_state;

}
unsigned int NexMatchmakeSession::WaitUpdateSessionSetting()
{
    return m_state;

}

}}}
