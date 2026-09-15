#include "nn/pia/lan/LanUpdateSessionSetting.hpp"
#include <cstring>
namespace nn { namespace pia { namespace lan {
LanUpdateSessionSetting::LanUpdateSessionSetting() : m_minParticipantNum(0xffff), m_maxParticipantNum(0xffff), m_sessionType(cLanSessionTypeInvalid), m_applicationDataSize(0) { std::memset(m_attributes, 0, sizeof(m_attributes)); std::memset(m_applicationData, 0, sizeof(m_applicationData)); }
LanUpdateSessionSetting::~LanUpdateSessionSetting() {}
unsigned short LanUpdateSessionSetting::GetMinParticipantNum() const { return m_minParticipantNum; }
unsigned short LanUpdateSessionSetting::GetMaxParticipantNum() const { return m_maxParticipantNum; }
LanSessionType LanUpdateSessionSetting::GetSessionType() const { return m_sessionType; }
unsigned int LanUpdateSessionSetting::GetAttribute(unsigned int index) const { return index < 6 ? m_attributes[index] : 0; }
Result LanUpdateSessionSetting::SetApplicationData(const void* data, unsigned int size) { if (data == 0 || size > sizeof(m_applicationData)) return 0x10c07; std::memset(m_applicationData, 0, sizeof(m_applicationData)); std::memcpy(m_applicationData, data, size); m_applicationDataSize = size; m_modificationFlag |= 2; return 0; }
Result LanUpdateSessionSetting::GetApplicationData(void* output, unsigned int size) const { if (output == 0) return 0x10c07; if (m_applicationDataSize == 0) return 0x10c08; if (size < m_applicationDataSize) return 0x10c04; std::memcpy(output, m_applicationData, m_applicationDataSize); return 0; }
unsigned int LanUpdateSessionSetting::GetApplicationDataSize() const { return m_applicationDataSize; }
void LanUpdateSessionSetting::Reset() { session::UpdateSessionSetting::Reset(); m_sessionType = cLanSessionTypeInvalid; m_minParticipantNum = 0xffff; m_maxParticipantNum = 0xffff; std::memset(m_attributes, 0, sizeof(m_attributes)); std::memset(m_applicationData, 0, sizeof(m_applicationData)); m_applicationDataSize = 0; }
void LanUpdateSessionSetting::Trace(unsigned long long) const {}
}}}
