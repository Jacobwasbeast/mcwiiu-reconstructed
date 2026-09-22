#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanCreateSessionSetting::LanCreateSessionSetting() : session::CreateSessionSetting() { Reset(); }
LanCreateSessionSetting::~LanCreateSessionSetting() {}
void LanCreateSessionSetting::SetGameMode(unsigned int value) { gameMode = value; stateFlags |= cLanSettingGameMode; }
unsigned int LanCreateSessionSetting::GetGameMode() const { return gameMode; }
bool LanCreateSessionSetting::IsSetGameMode() const { return (stateFlags & cLanSettingGameMode) != 0; }
void LanCreateSessionSetting::SetSessionType(LanSessionType value) { sessionType = value; stateFlags |= cLanSettingSessionType; }
LanSessionType LanCreateSessionSetting::GetSessionType() const { return sessionType; }
bool LanCreateSessionSetting::IsSetSessionType() const { return (stateFlags & cLanSettingSessionType) != 0; }
void LanCreateSessionSetting::SetAttribute(unsigned int index, unsigned int value) { if (index < cLanAttributeCount) { attributes[index] = value; stateFlags |= (cLanSettingAttribute0 << index); } }
unsigned int LanCreateSessionSetting::GetAttribute(unsigned int index) const { return index < cLanAttributeCount ? attributes[index] : 0; }
bool LanCreateSessionSetting::IsSetAttribute(unsigned int index) const { return index < cLanAttributeCount && (stateFlags & (cLanSettingAttribute0 << index)) != 0; }
void LanCreateSessionSetting::SetApplicationData(const void* data, unsigned int size) { if (data != 0 && size < cLanApplicationDataCapacity) { applicationDataSize = size; std::memcpy(applicationData, data, size); stateFlags |= cLanSettingApplicationData; } }
void LanCreateSessionSetting::GetApplicationData(void* output, unsigned int size) const { if (output != 0 && applicationDataSize != 0 && size >= applicationDataSize) std::memcpy(output, applicationData, applicationDataSize); }
unsigned int LanCreateSessionSetting::GetApplicationDataSize() const { return applicationDataSize; }
bool LanCreateSessionSetting::IsSetApplicationData() const { return (stateFlags & cLanSettingApplicationData) != 0; }
void LanCreateSessionSetting::SetOpenSession(bool value) { openSession = value; stateFlags |= cLanSettingOpenSession; }
bool LanCreateSessionSetting::IsOpenSession() const { return openSession; }
bool LanCreateSessionSetting::IsSetOpenSession() const { return (stateFlags & cLanSettingOpenSession) != 0; }
void LanCreateSessionSetting::Reset() { session::CreateSessionSetting::Reset(); gameMode = 0; sessionType = cLanSessionTypeInvalid; for (unsigned int& attribute : attributes) attribute = 0; std::memset(applicationData, 0, sizeof(applicationData)); applicationDataSize = 0; openSession = false; stateFlags = 0; }
void LanCreateSessionSetting::Trace(unsigned long long value) const { session::CreateSessionSetting::Trace(value); }
void LanCreateSessionSetting::Copy(const LanCreateSessionSetting& other) { gameMode = other.gameMode; sessionType = other.sessionType; std::memcpy(attributes, other.attributes, sizeof(attributes)); std::memcpy(applicationData, other.applicationData, sizeof(applicationData)); applicationDataSize = other.applicationDataSize; openSession = other.openSession; stateFlags = other.stateFlags; }
LanCreateSessionSetting& LanCreateSessionSetting::operator=(const LanCreateSessionSetting& other) { if (this != &other) Copy(other); return *this; }

}}}
