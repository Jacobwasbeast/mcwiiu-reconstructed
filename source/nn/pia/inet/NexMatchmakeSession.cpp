#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/nex/Variant.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/inet/NexFacade.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{

enum
{
    cResultInvalidArgument = 0x10c07
};

enum
{
    cGatheringFlagsHostMigration = 0x210,
    cGatheringFlagsMatchmake = 0x800
};

const wchar_t* s_countryCodeKey = L"@NCC";
const wchar_t* s_overrideIpAddressKey = L"@OIA";
const wchar_t* s_useGeoIpKey = L"@UsGI";
const wchar_t* s_ratingValueKey = L"@RV";
const wchar_t* s_disconnectionRateKey = L"@DR";
const wchar_t* s_violationRateKey = L"@VR";

}

NexMatchmakeSession::NexMatchmakeSession()
    : m_protocolCallContext(0),
      m_protocolCallContextB(0)
{
}

NexMatchmakeSession::~NexMatchmakeSession()
{
}

Result NexMatchmakeSession::BrowseMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return 0;
}

void NexMatchmakeSession::Cleanup()
{
}

unsigned long long NexMatchmakeSession::ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

Result NexMatchmakeSession::CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId)
{
    return 0;
}

SessionType NexMatchmakeSession::ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType)
{
    if (systemType == nn::nex::cMatchmakeSystemType_2) {
        return cSessionType_Ranked;
    }
    return cSessionType_Normal;
}

unsigned int NexMatchmakeSession::ConvertPersistentGatheringCodeToGatheringId(unsigned long long code)
{
    return 0;
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

nn::nex::MatchmakeSystemType NexMatchmakeSession::ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType)
{
    if (sessionType == cSessionType_Normal) {
        return nn::nex::cMatchmakeSystemType_1;
    }
    if (sessionType == cSessionType_Ranked) {
        return nn::nex::cMatchmakeSystemType_2;
    }
    return nn::nex::cMatchmakeSystemType_0;
}

void NexMatchmakeSession::CopyFoundPersistentGatheringToBrowsedCommunityInfoList(const nn::nex::PersistentGathering* gathering, NexCommunityInfo* communityInfo)
{
}

void NexMatchmakeSession::CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session, NexSessionInfo* sessionInfo)
{
}

unsigned long long NexMatchmakeSession::CreateCommunity(nn::pia::common::CallContext* callContext)
{
    return 0;
}

Result NexMatchmakeSession::CreateMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return 0;
}

Result NexMatchmakeSession::CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::Finalize()
{
    return 0;
}

unsigned long long NexMatchmakeSession::FindCommunityByGatheringId(nn::pia::common::CallContext* callContext)
{
    return 0;
}

unsigned long long NexMatchmakeSession::FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId, unsigned int offset, unsigned int size)
{
    return 0;
}

unsigned long long NexMatchmakeSession::FindCommunityByParticipant(nn::pia::common::CallContext* callContext, unsigned long long participantId, unsigned int offset, unsigned int size)
{
    return 0;
}

unsigned long long NexMatchmakeSession::FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isOfficial, unsigned int offset, unsigned int size)
{
    return 0;
}

Result NexMatchmakeSession::FindSessionByOwner(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaOwner& criteria)
{
    return 0;
}

Result NexMatchmakeSession::FindSessionByParticipant(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaParticipant& criteria)
{
    return 0;
}

Result NexMatchmakeSession::FindSessionBySessionId(nn::pia::common::CallContext* callContext, const NexSessionSearchCriteriaSessionId& criteria)
{
    return 0;
}

unsigned long long NexMatchmakeSession::GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

unsigned int NexMatchmakeSession::GetBrowsedCommunityResult()
{
    return m_browsedCommunityResult;
}

session::ISessionInfoList* NexMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return m_browseMatchmakeSessionResult;
}

unsigned int NexMatchmakeSession::GetRandomMatchmakeScore() const
{
    return m_randomMatchmakeScore;
}

Result NexMatchmakeSession::GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::Initialize()
{
    return 0;
}

unsigned char NexMatchmakeSession::IsMatchmakeHost() const
{
    return m_matchmakeHost;
}

unsigned char NexMatchmakeSession::IsProcessingJoinMatchmakeSession() const
{
    return m_processingJoinMatchmakeSession;
}

unsigned char NexMatchmakeSession::IsRandomMatchmakeScoreAvailable() const
{
    return m_randomMatchmakeScoreAvailable;
}

unsigned int NexMatchmakeSession::IsUpdatableSelectionPriority(unsigned char priority)
{
    return 0;
}

unsigned long long NexMatchmakeSession::JoinCommunity(nn::pia::common::CallContext* callContext)
{
    return 0;
}

Result NexMatchmakeSession::JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

Result NexMatchmakeSession::JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int teamIndex)
{
    return 0;
}

Result NexMatchmakeSession::LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned long long* candidateIds, unsigned short candidateCount)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int index, unsigned int value)
{
    return 0;
}

Result NexMatchmakeSession::OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

Result NexMatchmakeSession::RandomMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    return 0;
}

Result NexMatchmakeSession::RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned long long* teamOwnerIds, unsigned short teamCount, unsigned int gatheringId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ReflectCommunityIdList(const unsigned int* idList, unsigned int count)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting)
{
    return 0;
}

void NexMatchmakeSession::ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting, bool isTeam)
{
}

unsigned long long NexMatchmakeSession::ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting, bool isTeam)
{
    return 0;
}

unsigned long long NexMatchmakeSession::ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting)
{
    return 0;
}

Result NexMatchmakeSession::ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting, bool hostMigrationEnabled)
{
    if (setting == 0) {
        return static_cast<Result>(cResultInvalidArgument);
    }
    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(setting);
    m_requestSession->Reset();
    if (createSetting->IsSetGameMode()) {
        m_requestSession->SetGameMode(createSetting->GetGameMode());
    }
    if (createSetting->IsSetMaxParticipantNum()) {
        m_requestSession->m_uiShortValue2 = createSetting->GetMaxParticipantNum();
    }
    if (createSetting->IsSetMinParticipantNum()) {
        m_requestSession->m_uiShortValue1 = createSetting->GetMinParticipantNum();
    }
    nn::nex::MatchmakeSystemType systemType = nn::nex::cMatchmakeSystemType_1;
    unsigned int policyArgument = 0;
    if (createSetting->IsSetSessionType()) {
        systemType = ConvertSessionTypeToMatchmakeSystemType(createSetting->GetSessionType());
        if (systemType == nn::nex::cMatchmakeSystemType_1) {
            if (createSetting->IsAutoCloseSession()) {
                policyArgument = 1;
            } else {
                policyArgument = 0;
            }
        } else if (systemType == nn::nex::cMatchmakeSystemType_2) {
            if (createSetting->IsAutoCloseSession()) {
                policyArgument = 0;
            } else {
                policyArgument = 1;
            }
        }
    }
    m_requestSession->SetMatchmakeSystemType(systemType, policyArgument);
    if (createSetting->IsSetSessionUserPassword()) {
        common::String::EncodingType encoding = createSetting->GetSessionUserPassword().GetEncodingType();
        nn::nex::MatchmakeSession* session = m_requestSession;
        if (encoding == common::String::EncodingType_Utf16) {
            session->m_strUserPassword = nn::nex::String(static_cast<const wchar_t*>(createSetting->GetSessionUserPassword().GetBuffer()));
        } else {
            session->m_strUserPassword = nn::nex::String(static_cast<const char*>(createSetting->GetSessionUserPassword().GetBuffer()));
        }
    }
    if (createSetting->IsSetOpenSession()) {
        m_requestSession->SetOpenParticipation(createSetting->IsOpenSession());
    }
    if (createSetting->IsSetSelectionPriority()) {
        m_requestSession->SetProgressScore(createSetting->GetSelectionPriority());
    }
    if (createSetting->IsSetCommunityId()) {
        m_requestSession->m_uiReferGatheringID = createSetting->GetCommunityId();
    }
    m_matchmakeParam->m_Parameters.clear();
    if (createSetting->IsSetRatingValue()) {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_ratingValueKey)] = nn::nex::Variant(createSetting->GetRatingValue());
    }
    if (createSetting->IsSetDisconnectionRate()) {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_disconnectionRateKey)] = nn::nex::Variant(createSetting->GetDisconnectionRate());
    }
    if (createSetting->IsSetViolationRate()) {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_violationRateKey)] = nn::nex::Variant(createSetting->GetViolationRate());
    }
    if (createSetting->IsSetUseGeoIp()) {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_useGeoIpKey)] = nn::nex::Variant(createSetting->GetUseGeoIp());
    }
    if (createSetting->IsSetCountryCode()) {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_countryCodeKey)] = nn::nex::Variant(createSetting->GetCountryCode());
    }
    if (createSetting->IsSetOverrideIpAddress()) {
        NexFacade::ConvertInetAddressToNexInetAddress(createSetting->GetOverrideIpAddress(), m_inetAddress);
        m_matchmakeParam->m_Parameters[nn::nex::String(s_overrideIpAddressKey)] = nn::nex::Variant(m_inetAddress->GetAddressStr());
    }
    if (createSetting->GetAdditionalScoreBasedParameterStringKey(0) != 0 && createSetting->GetAdditionalScoreBasedParameterString(0) != 0) {
        m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(createSetting->GetAdditionalScoreBasedParameterStringKey(0)->GetBuffer()))] = nn::nex::Variant(static_cast<const wchar_t*>(createSetting->GetAdditionalScoreBasedParameterString(0)->GetBuffer()));
    }
    if (createSetting->IsSetScoreBasedParameter()) {
        m_requestSession->m_MatchmakeParam = *m_matchmakeParam;
    }
    common::g_sessionBeginMonitoringContent.m_metric81 = createSetting->GetGameMode();
    common::g_sessionBeginMonitoringContent.m_metric83 = createSetting->GetMaxParticipantNum();
    common::g_sessionBeginMonitoringContent.m_metric108 = createSetting->GetExtraParticipantNum();
    common::g_sessionStateMonitoringContent.m_metric223[1] = createSetting->IsOpenSession();
    if (createSetting->IsSetApplicationData()) {
        nn::nex::qVector<unsigned char> applicationData;
        createSetting->CopyApplicationDataToVector(&applicationData);
        m_requestSession->SetApplicationBuffer(applicationData);
    }
    for (unsigned int i = 0; i < NexCreateSessionSetting::cAttributeNum; ++i) {
        if (createSetting->IsSetAttribute(i)) {
            m_requestSession->SetAttribute(i, createSetting->GetAttribute(i));
            common::g_sessionBeginMonitoringContent.m_metric82[i] = createSetting->GetAttribute(i);
        }
    }
    unsigned int flags = 0;
    if (hostMigrationEnabled) {
        flags = cGatheringFlagsHostMigration;
    }
    m_requestSession->m_uiValue4 |= flags | cGatheringFlagsMatchmake;
    return static_cast<Result>(0);
}

unsigned long long NexMatchmakeSession::ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting)
{
    return 0;
}

Result NexMatchmakeSession::ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting, bool isTeam)
{
    return 0;
}

void NexMatchmakeSession::RegisterFindCommunityResult()
{
}

void NexMatchmakeSession::RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType)
{
}

unsigned long long NexMatchmakeSession::RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria, unsigned char flags)
{
    return 0;
}

Result NexMatchmakeSession::RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

unsigned long long NexMatchmakeSession::RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

void NexMatchmakeSession::SetMatchmakeCurrentParticipantNum(unsigned short participantNum)
{
}

void NexMatchmakeSession::SetMatchmakeOwnerUniqueId(unsigned long long ownerId)
{
    CommonMatchmakeSession::SetMatchmakeOwnerUniqueId(ownerId);
}

unsigned long long NexMatchmakeSession::SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize)
{
    return 0;
}

unsigned long long NexMatchmakeSession::SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key)
{
    return 0;
}

void NexMatchmakeSession::SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId)
{
    m_temporaryOwnerChangeGatheringId = gatheringId;
    m_temporaryOwnerChangeOwnerId = ownerId;
}

Result NexMatchmakeSession::UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

void NexMatchmakeSession::UnregisterSearchCriteria()
{
    m_searchCriteriaList->clear();
    m_resultRangeList->clear();
}

unsigned long long NexMatchmakeSession::UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId, const unsigned char* buffer, unsigned int size)
{
    return 0;
}

unsigned long long NexMatchmakeSession::UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int* attributes)
{
    return 0;
}

unsigned long long NexMatchmakeSession::UpdateCommunity(nn::pia::common::CallContext* callContext)
{
    return 0;
}

Result NexMatchmakeSession::UpdateMatchmakeSessionHost(unsigned int stationId)
{
    return 0;
}

Result NexMatchmakeSession::UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority)
{
    return 0;
}

Result NexMatchmakeSession::UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool NexMatchmakeSession::WaitBrowseMatchmakeSession()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitClearSystemPassword()
{
    return 0;
}

bool NexMatchmakeSession::WaitCloseParticipate()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitCreateCommunity()
{
    return 0;
}

bool NexMatchmakeSession::WaitCreateMatchmakeSession(unsigned int* gatheringId)
{
    return false;
}

bool NexMatchmakeSession::WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId)
{
    return false;
}

unsigned int NexMatchmakeSession::WaitFindCommunityByGatheringId()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitFindCommunityByOwner()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitFindCommunityByParticipant()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitFindOfficialCommunity()
{
    return 0;
}

bool NexMatchmakeSession::WaitFindSessionByOwner()
{
    return false;
}

bool NexMatchmakeSession::WaitFindSessionByParticipant()
{
    return false;
}

bool NexMatchmakeSession::WaitFindSessionBySessionId()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitGenerateSystemPassword()
{
    return 0;
}

bool NexMatchmakeSession::WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info)
{
    return false;
}

unsigned int NexMatchmakeSession::WaitJoinCommunity()
{
    return 0;
}

bool NexMatchmakeSession::WaitJoinMatchmakeSession()
{
    return false;
}

bool NexMatchmakeSession::WaitJoinMatchmakeSessionWithTeam()
{
    return false;
}

bool NexMatchmakeSession::WaitLeaveMatchmakeSession()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitLeaveMatchmakeSessionCompanion()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitMigrateMatchmakeSessionOwner()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitModifyAttribute()
{
    return 0;
}

bool NexMatchmakeSession::WaitOpenParticipate()
{
    return false;
}

bool NexMatchmakeSession::WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found)
{
    return false;
}

bool NexMatchmakeSession::WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found)
{
    return false;
}

bool NexMatchmakeSession::WaitRequestSessionInfo()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitRequestSessionInfoWithoutPermission()
{
    return 0;
}

bool NexMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    return false;
}

unsigned int NexMatchmakeSession::WaitUpdateApplicationBuffer()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitUpdateAttributes()
{
    return 0;
}

unsigned int NexMatchmakeSession::WaitUpdateCommunity()
{
    return 0;
}

bool NexMatchmakeSession::WaitUpdateSessionSetting()
{
    return false;
}

}}}
