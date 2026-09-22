#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKESESSION_HPP

#include <cstdint>

namespace nn { namespace pia { namespace lan {

class LanMatchmakeSession
{
public:
    LanMatchmakeSession();
    ~LanMatchmakeSession();
    void Clear();
    void Cleanup();
    uint64_t RandomMatchmakeSession(uint32_t a1, uint32_t a2);
    void SetSessionParam(void* a1, uint32_t a2);
    uint32_t WaitRandomMatchmakeSession(void* a1, void* a2);
    uint64_t BrowseMatchmakeSession(uint32_t a1, uint32_t a2);
    uint32_t WaitBrowseMatchmakeSession();
    uint64_t CreateMatchmakeSession(uint32_t a1, uint32_t a2);
    uint32_t WaitCreateMatchmakeSession(void* a1);
    uint64_t JoinMatchmakeSession(uint32_t a1, uint32_t a2, uint32_t a3);
    uint32_t WaitJoinMatchmakeSession();
    uint64_t LeaveMatchmakeSession(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitLeaveMatchmakeSession();
    uint64_t UnregisterMatchmakeSession(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitUnregisterMatchmakeSession();
    uint64_t OpenParticipate(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitOpenParticipate();
    uint64_t CloseParticipate(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitCloseParticipate();
    uint64_t GetStationConnectionInfo(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitGetStationConnectionInfo(void* a1);
    uint64_t UpdateSessionSetting(void* a1, uint32_t a2);
    uint32_t WaitUpdateSessionSetting();
    uint64_t RequestSessionInfo(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitRequestSessionInfo();
    uint64_t InquiryOwnerPrincipalId(uint32_t a1, uint32_t a2, int32_t a3);
    uint32_t WaitInquiryOwnerPrincipalId();
    uint64_t CancelInquiryOwnerPrincipalId();
    uint32_t GetGatheringId();
    int32_t GetSessionInfoPtr();
    int32_t GetSessionInfoBufferPtr();
    uint32_t GetSessionInfoListPtr();
    int32_t GetSessionSearchCriteriaPtr(uint8_t a1);
    void ReflectSettingToMatchmakeSession(void* a1);
    void ReflectSettingToMatchmakeSession(const unsigned char* a1);
    uint64_t ReflectSettingToMatchmakeSession(const unsigned char* a1, bool a2);
    uint32_t RegisterSearchCriteria(void* a1, uint8_t a2);
    void UnregisterSearchCriteria();
    uint64_t UpdateStationInfoList();
    void SetIgnoredSessionId(uint32_t a1);
    void SetPreferredSessionId(uint32_t a1);
    int32_t GetSessionInfo();
    void CopySessionInfo();
    uint8_t IsSessionParamRegistered();
    uint32_t GetBrowseMatchmakeSessionResult();
    uint64_t UpdateMatchmakeSessionHost(uint32_t a1);

private:
    unsigned char m_storage[6912];
};

}}}

#endif
