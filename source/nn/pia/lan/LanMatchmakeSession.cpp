#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cResultNotSupported = 0xc0b
};
}

LanMatchmakeSession::LanMatchmakeSession()
{
    m_sessionInfoList = session::g_piaSession->m_sessionInfoList;
    Clear();
}

LanMatchmakeSession::~LanMatchmakeSession()
{
    m_sessionInfoList = 0;
}

void LanMatchmakeSession::Clear()
{
    m_sessionInfo.Clear();
    m_sessionInfoBuffer.Clear();
    for (unsigned int i = 0; i < 2; ++i)
        m_searchCriteria[i].Reset();
    m_sessionInfoList->Clear();
    session::CommonMatchmakeSession::Cleanup();
    m_inquiryCallContext = 0;
    m_ignoredSessionId = 0;
    m_preferredSessionId = 0;
    m_matchmakeOwnerUniqueId = 0;
    m_sessionParamRegistered = false;
    m_matchmakeOwner = false;
    m_gatheringId = 0;
    m_callContext = 0;
}

void LanMatchmakeSession::Cleanup()
{
    if (m_callContext != 0)
        m_callContext->SignalCancel();
    if (m_inquiryCallContext != 0)
        m_inquiryCallContext->SignalCancel();
    if (m_ownedCallContext.m_state == common::CallContext::cStateInProgress)
        m_ownedCallContext.SignalCancel();
    Clear();
}

Result LanMatchmakeSession::RandomMatchmakeSession(common::CallContext* callContext)
{
    return 0;
}

void LanMatchmakeSession::SetSessionParam(const unsigned char* sessionParam, unsigned int sessionParamSize)
{
}

bool LanMatchmakeSession::WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found)
{
    return false;
}

Result LanMatchmakeSession::BrowseMatchmakeSession(common::CallContext* callContext)
{
    return 0;
}

bool LanMatchmakeSession::WaitBrowseMatchmakeSession()
{
    return false;
}

Result LanMatchmakeSession::CreateMatchmakeSession(common::CallContext* callContext)
{
    return 0;
}

bool LanMatchmakeSession::WaitCreateMatchmakeSession(unsigned int* gatheringId)
{
    return false;
}

Result LanMatchmakeSession::JoinMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitJoinMatchmakeSession()
{
    return false;
}

Result LanMatchmakeSession::LeaveMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitLeaveMatchmakeSession()
{
    return false;
}

Result LanMatchmakeSession::UnregisterMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    m_callContext = callContext;
    m_sessionInfo.Clear();
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    return false;
}

Result LanMatchmakeSession::OpenParticipate(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitOpenParticipate()
{
    return false;
}

Result LanMatchmakeSession::CloseParticipate(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitCloseParticipate()
{
    return false;
}

Result LanMatchmakeSession::GetStationConnectionInfo(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitGetStationConnectionInfo(transport::StationConnectionInfo* stationConnectionInfo)
{
    return false;
}

Result LanMatchmakeSession::RequestSessionInfo(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitRequestSessionInfo()
{
    return false;
}

Result LanMatchmakeSession::UpdateMatchmakeSessionHost(unsigned int stationId)
{
    return cResultNotSupported;
}

void LanMatchmakeSession::UpdateStationInfoList()
{
}

const LanSessionInfo* LanMatchmakeSession::GetSessionInfo() const
{
    return &m_publishedSessionInfo;
}

LanSessionInfo* LanMatchmakeSession::GetSessionInfo()
{
    return &m_publishedSessionInfo;
}

Result LanMatchmakeSession::UpdateSessionSetting(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitUpdateSessionSetting()
{
    return false;
}

Result LanMatchmakeSession::InquiryOwnerPrincipalId(common::CallContext* callContext, unsigned int gatheringId)
{
    return 0;
}

bool LanMatchmakeSession::WaitInquiryOwnerPrincipalId()
{
    return false;
}

Result LanMatchmakeSession::CancelInquiryOwnerPrincipalId()
{
    if (m_inquiryCallContext != 0)
    {
        m_inquiryCallContext->SignalCancel();
        m_inquiryCallContext = 0;
    }
    return 0;
}

unsigned int LanMatchmakeSession::GetGatheringId() const
{
    return m_gatheringId;
}

LanSessionInfo* LanMatchmakeSession::GetSessionInfoPtr()
{
    return &m_sessionInfo;
}

LanSessionInfo* LanMatchmakeSession::GetSessionInfoBufferPtr()
{
    return &m_sessionInfoBuffer;
}

session::ISessionInfoList* LanMatchmakeSession::GetSessionInfoListPtr()
{
    return m_sessionInfoList;
}

session::ISessionInfoList* LanMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return m_sessionInfoList;
}

LanSessionSearchCriteria* LanMatchmakeSession::GetSessionSearchCriteriaPtr(unsigned char index)
{
    if (index >= 2)
        return 0;
    return &m_searchCriteria[index];
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::CreateSessionSetting* setting)
{
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::JoinSessionSetting* setting)
{
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::UpdateSessionSetting* setting, bool open)
{
}

unsigned int LanMatchmakeSession::RegisterSearchCriteria(const session::SessionSearchCriteria* criteria, unsigned char count)
{
    m_searchCriteriaCount = count > 2 ? 2 : count;
    const LanSessionSearchCriteria* source = static_cast<const LanSessionSearchCriteria*>(criteria);
    for (unsigned char i = 0; i < m_searchCriteriaCount; ++i)
        m_searchCriteria[i] = *source++;
    return 1;
}

void LanMatchmakeSession::UnregisterSearchCriteria()
{
    for (unsigned int i = 0; i < 2; ++i)
        m_searchCriteria[i].Reset();
    m_searchCriteriaCount = 0;
}

void LanMatchmakeSession::SetIgnoredSessionId(unsigned int sessionId)
{
    m_ignoredSessionId = sessionId;
}

void LanMatchmakeSession::SetPreferredSessionId(unsigned int sessionId)
{
    m_preferredSessionId = sessionId;
}

void LanMatchmakeSession::CopySessionInfo()
{
}

bool LanMatchmakeSession::IsSessionParamRegistered() const
{
    return m_sessionParamRegistered;
}

void LanMatchmakeSession::Trace(unsigned long long value) const
{
}

}}}
