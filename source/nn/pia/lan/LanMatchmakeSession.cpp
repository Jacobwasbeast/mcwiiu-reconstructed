#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanMatchmakeSession::LanMatchmakeSession()
{
}

LanMatchmakeSession::~LanMatchmakeSession()
{
}

void LanMatchmakeSession::Clear()
{
}

void LanMatchmakeSession::Cleanup()
{
}

uint64_t LanMatchmakeSession::RandomMatchmakeSession(uint32_t a1, uint32_t a2)
{
    return 0;
}

void LanMatchmakeSession::SetSessionParam(void* a1, uint32_t a2)
{
}

uint32_t LanMatchmakeSession::WaitRandomMatchmakeSession(void* a1, void* a2)
{
    return 0;
}

uint64_t LanMatchmakeSession::BrowseMatchmakeSession(uint32_t a1, uint32_t a2)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitBrowseMatchmakeSession()
{
    return 0;
}

uint64_t LanMatchmakeSession::CreateMatchmakeSession(uint32_t a1, uint32_t a2)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitCreateMatchmakeSession(void* a1)
{
    return 0;
}

uint64_t LanMatchmakeSession::JoinMatchmakeSession(uint32_t a1, uint32_t a2, uint32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitJoinMatchmakeSession()
{
    return 0;
}

uint64_t LanMatchmakeSession::LeaveMatchmakeSession(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitLeaveMatchmakeSession()
{
    return 0;
}

uint64_t LanMatchmakeSession::UnregisterMatchmakeSession(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    return 0;
}

uint64_t LanMatchmakeSession::OpenParticipate(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitOpenParticipate()
{
    return 0;
}

uint64_t LanMatchmakeSession::CloseParticipate(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitCloseParticipate()
{
    return 0;
}

uint64_t LanMatchmakeSession::GetStationConnectionInfo(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitGetStationConnectionInfo(void* a1)
{
    return 0;
}

uint64_t LanMatchmakeSession::UpdateSessionSetting(void* a1, uint32_t a2)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitUpdateSessionSetting()
{
    return 0;
}

uint64_t LanMatchmakeSession::RequestSessionInfo(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitRequestSessionInfo()
{
    return 0;
}

uint64_t LanMatchmakeSession::InquiryOwnerPrincipalId(uint32_t a1, uint32_t a2, int32_t a3)
{
    return 0;
}

uint32_t LanMatchmakeSession::WaitInquiryOwnerPrincipalId()
{
    return 0;
}

uint64_t LanMatchmakeSession::CancelInquiryOwnerPrincipalId()
{
    return 0;
}

uint32_t LanMatchmakeSession::GetGatheringId()
{
    uint32_t value = 0;
    std::memcpy(&value, m_storage + 0x1378, sizeof(value));
    return value;
}

int32_t LanMatchmakeSession::GetSessionInfoPtr()
{
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(this) + 0x18);
}

int32_t LanMatchmakeSession::GetSessionInfoBufferPtr()
{
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(this) + 0x770);
}

uint32_t LanMatchmakeSession::GetSessionInfoListPtr()
{
    uint32_t value = 0;
    std::memcpy(&value, m_storage + 0x1374, sizeof(value));
    return value;
}

int32_t LanMatchmakeSession::GetSessionSearchCriteriaPtr(uint8_t a1)
{
    if (a1 >= 2)
        return 0;
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(this) + 0xec8 + a1 * 0x254);
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(void* a1)
{
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const unsigned char* a1)
{
}

uint64_t LanMatchmakeSession::ReflectSettingToMatchmakeSession(const unsigned char* a1, bool a2)
{
    return 0;
}

uint32_t LanMatchmakeSession::RegisterSearchCriteria(void* a1, uint8_t a2)
{
    return 0;
}

void LanMatchmakeSession::UnregisterSearchCriteria()
{
}

uint64_t LanMatchmakeSession::UpdateStationInfoList()
{
    return 0;
}

void LanMatchmakeSession::SetIgnoredSessionId(uint32_t a1)
{
}

void LanMatchmakeSession::SetPreferredSessionId(uint32_t a1)
{
}

int32_t LanMatchmakeSession::GetSessionInfo()
{
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(this) + 0x13a0);
}

void LanMatchmakeSession::CopySessionInfo()
{
}

uint8_t LanMatchmakeSession::IsSessionParamRegistered()
{
    return 0;
}

uint32_t LanMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return 0;
}

uint64_t LanMatchmakeSession::UpdateMatchmakeSessionHost(uint32_t a1)
{
    return 0;
}

}}}
