#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include <cstring>

namespace nn { namespace pia { namespace inet {

NexCreateSessionSetting::NexCreateSessionSetting() : m_applicationData() { Reset(); }
NexCreateSessionSetting::NexCreateSessionSetting(const NexCreateSessionSetting& other) : m_applicationData() { Copy(other); }
NexCreateSessionSetting::~NexCreateSessionSetting() {}
NexCreateSessionSetting& NexCreateSessionSetting::operator=(const NexCreateSessionSetting& other) { Copy(other); return *this; }
void NexCreateSessionSetting::Copy(const NexCreateSessionSetting& other) { std::memcpy(this, &other, sizeof(*this)); }
void NexCreateSessionSetting::SetGameMode(unsigned int value) { m_gameMode = value; m_stateFlags |= 1u; }
unsigned int NexCreateSessionSetting::GetGameMode() const { return m_gameMode; }
bool NexCreateSessionSetting::IsSetGameMode() const { return (m_stateFlags & 1u) != 0; }
void NexCreateSessionSetting::SetSessionType(SessionType value) { m_sessionType = value; m_autoCloseSession = value == cSessionType_Ranked; m_stateFlags |= 2u; }
SessionType NexCreateSessionSetting::GetSessionType() const { return m_sessionType; }
bool NexCreateSessionSetting::IsSetSessionType() const { return (m_stateFlags & 2u) != 0; }
bool NexCreateSessionSetting::IsAutoCloseSession() const { return m_autoCloseSession; }
void NexCreateSessionSetting::SetOpenSession(bool value) { m_autoCloseSession = !value; m_stateFlags |= 4u; }
bool NexCreateSessionSetting::IsOpenSession() const { return !m_autoCloseSession; }
bool NexCreateSessionSetting::IsSetOpenSession() const { return (m_stateFlags & 4u) != 0; }
bool NexCreateSessionSetting::SetAttribute(unsigned int index, unsigned int value) { if (index >= 6 || IsSetSessionMatchmakeKeyword()) return false; m_attributes[index] = value; m_stateFlags |= (1u << (index + 2)); return true; }
unsigned int NexCreateSessionSetting::GetAttribute(unsigned int index) const { return index < 6 ? m_attributes[index] : 0; }
bool NexCreateSessionSetting::IsSetAttribute(unsigned int index) const { return index < 6 && (m_stateFlags & (1u << (index + 2))) != 0; }
bool NexCreateSessionSetting::SetApplicationData(const void* data, unsigned int size) { if (data == 0 || size == 0 || size > sizeof(m_applicationData)) return false; std::memcpy(m_applicationData, data, size); m_applicationDataSize = size; m_stateFlags |= 0x100u; return true; }
bool NexCreateSessionSetting::GetApplicationData(void* data, unsigned int size) const { if (data == 0 || size < m_applicationDataSize) return false; std::memcpy(data, m_applicationData, m_applicationDataSize); return true; }
unsigned int NexCreateSessionSetting::GetApplicationDataSize() const { return m_applicationDataSize; }
bool NexCreateSessionSetting::IsSetApplicationData() const { return (m_stateFlags & 0x100u) != 0; }
unsigned int NexCreateSessionSetting::GetMatchmakeSessionOption() const { return m_matchmakeSessionOption; }
bool NexCreateSessionSetting::IsSetMatchmakeSessionOption() const { return (m_stateFlags & 0x200u) != 0; }
unsigned char NexCreateSessionSetting::GetSelectionPriority() const { return m_selectionPriority; }
bool NexCreateSessionSetting::IsSetSelectionPriority() const { return (m_stateFlags & 0x400u) != 0; }
unsigned int NexCreateSessionSetting::GetRatingValue() const { return m_ratingValue; }
bool NexCreateSessionSetting::IsSetRatingValue() const { return (m_stateFlags & 0x800u) != 0; }
unsigned int NexCreateSessionSetting::GetDisconnectionRate() const { return m_disconnectionRate; }
bool NexCreateSessionSetting::IsSetDisconnectionRate() const { return (m_stateFlags & 0x1000u) != 0; }
unsigned int NexCreateSessionSetting::GetViolationRate() const { return m_violationRate; }
bool NexCreateSessionSetting::IsSetViolationRate() const { return (m_stateFlags & 0x2000u) != 0; }
bool NexCreateSessionSetting::GetUseGeoIp() const { return m_useGeoIp; }
bool NexCreateSessionSetting::IsSetUseGeoIp() const { return m_useGeoIp; }
unsigned int NexCreateSessionSetting::GetCountryCode() const { return m_countryCode; }
bool NexCreateSessionSetting::IsSetCountryCode() const { return (m_stateFlags & 0x8000u) != 0; }
unsigned int NexCreateSessionSetting::GetExtraParticipantNum() const { return m_extraParticipantNum; }
bool NexCreateSessionSetting::IsSetExtraParticipantNum() const { return m_extraParticipantNum != 0; }
unsigned int NexCreateSessionSetting::GetCommunityId() const { return m_communityId; }
bool NexCreateSessionSetting::IsSetCommunityId() const { return (m_stateFlags & 0x40000000u) != 0; }
bool NexCreateSessionSetting::IsSetScoreBasedParameter() const { return (m_stateFlags & 0x2000000u) != 0; }
bool NexCreateSessionSetting::IsSetSessionMatchmakeKeyword() const { return (m_stateFlags & 0x400000u) != 0; }
void NexCreateSessionSetting::Reset() { m_gameMode = 0; m_sessionType = cSessionType_Normal; m_autoCloseSession = false; m_applicationDataSize = 0; m_matchmakeSessionOption = 0; m_selectionPriority = 0; m_ratingValue = 0; m_disconnectionRate = 0; m_violationRate = 0; m_useGeoIp = false; m_countryCode = 0; m_extraParticipantNum = 0; m_communityId = 0; m_stateFlags = 0; std::memset(m_attributes, 0, sizeof(m_attributes)); std::memset(m_applicationData, 0, sizeof(m_applicationData)); }

}}}
