#include "nn/pia/inet/NexSessionInfo.hpp"
#include <cstring>

namespace nn { namespace pia { namespace inet {

namespace {
const Result cResultInvalidArgument = static_cast<Result>(0x10c07);
const Result cResultBufferTooSmall = static_cast<Result>(0x10c04);
}

NexSessionInfo::NexSessionInfo() : game_mode_(0), session_id_(0),
    current_participant_num_(0), min_participant_num_(0), max_participant_num_(0),
    opened_(false), attributes_{}, application_data_{}, application_data_size_(0),
    user_password_enabled_(false), system_password_enabled_(false), session_type_(cSessionType_Normal),
    auto_close_session_(false), host_principal_id_(0), target_principal_id_(0),
    selection_priority_(0), started_time_(), community_id_(0) {}

NexSessionInfo::~NexSessionInfo() = default;

uint32_t NexSessionInfo::GetGameMode() const
{
    return game_mode_;
}

uint32_t NexSessionInfo::GetSessionId() const
{
    return session_id_;
}

uint32_t NexSessionInfo::GetCommunityId() const
{
    return community_id_;
}

uint16_t NexSessionInfo::GetCurrentParticipantNum() const
{
    return current_participant_num_;
}

uint16_t NexSessionInfo::GetMinParticipantNum() const
{
    return min_participant_num_;
}

uint16_t NexSessionInfo::GetMaxParticipantNum() const
{
    return max_participant_num_;
}

uint8_t NexSessionInfo::IsOpened() const
{
    return opened_;
}

Result NexSessionInfo::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == nullptr) return cResultInvalidArgument;
    if (bufferSize < application_data_size_) return cResultBufferTooSmall;
    std::memcpy(buffer, application_data_, application_data_size_);
    return 0;
}

uint32_t NexSessionInfo::GetApplicationDataSize() const
{
    return application_data_size_;
}

Result NexSessionInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (value == nullptr || index >= 6) return cResultInvalidArgument;
    *value = attributes_[index];
    return 0;
}

uint8_t NexSessionInfo::IsRestrictedByUserPassword() const
{
    return user_password_enabled_;
}

uint8_t NexSessionInfo::IsRestrictedBySystemPassword() const
{
    return system_password_enabled_;
}

uint32_t NexSessionInfo::GetSessionType() const
{
    return session_type_;
}

uint8_t NexSessionInfo::IsAutoCloseSession() const
{
    return auto_close_session_;
}

unsigned long long NexSessionInfo::GetHostPrincipalId() const
{
    return host_principal_id_;
}

unsigned long long NexSessionInfo::GetTargetPrincipalId() const
{
    return target_principal_id_;
}

uint8_t NexSessionInfo::GetSelectionPriority() const
{
    return selection_priority_;
}

const common::DateTime& NexSessionInfo::GetStartedTime() const
{
    return started_time_;
}

void NexSessionInfo::Clear()
{
    game_mode_ = session_id_ = community_id_ = 0;
    current_participant_num_ = min_participant_num_ = max_participant_num_ = 0;
    opened_ = user_password_enabled_ = system_password_enabled_ = auto_close_session_ = false;
    session_type_ = cSessionType_Normal; host_principal_id_ = target_principal_id_ = 0;
    selection_priority_ = 0; application_data_size_ = 0;
    for (uint32_t& attribute : attributes_) attribute = 0;
    started_time_ = common::DateTime();
    std::memset(application_data_, 0, sizeof(application_data_));
}

void NexSessionInfo::Trace(unsigned long long traceId) const
{
    (void)traceId;
}

void NexSessionInfo::SetGameMode(uint32_t gameMode)
{
    game_mode_ = gameMode;
}

void NexSessionInfo::SetSessionId(uint32_t sessionId)
{
    session_id_ = sessionId;
}

void NexSessionInfo::SetCurrentParticipantNum(uint16_t participantNum)
{
    current_participant_num_ = participantNum;
}

void NexSessionInfo::SetMinParticipantNum(uint16_t participantNum)
{
    min_participant_num_ = participantNum;
}

void NexSessionInfo::SetMaxParticipantNum(uint16_t participantNum)
{
    max_participant_num_ = participantNum;
}

void NexSessionInfo::SetOpen(bool open)
{
    opened_ = open;
}

void NexSessionInfo::SetAttribute(uint8_t index, uint32_t value)
{
    if (index < 6) attributes_[index] = value;
}

Result NexSessionInfo::SetApplicationData(const void* data, unsigned int size)
{
    if (data != nullptr && size <= sizeof(application_data_)) {
        std::memset(application_data_, 0, sizeof(application_data_));
        std::memcpy(application_data_, data, size);
        application_data_size_ = size;
    }
    return 0;
}

void NexSessionInfo::SetUserPasswordEnabled(bool enabled)
{
    user_password_enabled_ = enabled;
}

void NexSessionInfo::SetSystemPasswordEnabled(bool enabled)
{
    system_password_enabled_ = enabled;
}

void NexSessionInfo::SetSessionType(SessionType sessionType, bool autoCloseSession)
{
    session_type_ = sessionType; auto_close_session_ = autoCloseSession;
}

void NexSessionInfo::SetHostPrincipalId(unsigned long long principalId)
{
    host_principal_id_ = principalId;
}

void NexSessionInfo::SetTargetPrincipalId(unsigned long long principalId)
{
    target_principal_id_ = principalId;
}

void NexSessionInfo::SetSelectionPriority(uint8_t priority)
{
    selection_priority_ = priority;
}

void NexSessionInfo::SetStartedTime(const common::DateTime& time)
{
    started_time_ = time;
}

void NexSessionInfo::SetCommunityId(uint32_t communityId)
{
    community_id_ = communityId;
}

}}}
