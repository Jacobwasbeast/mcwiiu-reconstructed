#include "nn/pia/inet/NexJoinSessionSetting.hpp"

namespace nn { namespace pia { namespace inet {

NexJoinSessionSetting::NexJoinSessionSetting()
    : session::JoinSessionSetting(),
      m_sessionUserPassword(),
      m_sessionSystemPassword(),
      m_extraParticipantNum(0),
      m_matchmakeSessionOption(0),
      m_isSetMatchmakeSessionOption(false),
      m_isMyBlockListUsed(true),
      m_isOthersBlockListUsed(true)
{
}

NexJoinSessionSetting::~NexJoinSessionSetting()
{
}

void NexJoinSessionSetting::Reset()
{
    session::JoinSessionSetting::Reset();
    m_isSetMatchmakeSessionOption = false;
    m_isMyBlockListUsed = true;
    m_isOthersBlockListUsed = true;
    m_extraParticipantNum = 0;
    m_matchmakeSessionOption = 0;
    m_sessionUserPassword.Clear();
    m_sessionSystemPassword.Clear();
}

const common::String& NexJoinSessionSetting::GetSessionUserPassword() const
{
    return m_sessionUserPassword;
}

const common::String& NexJoinSessionSetting::GetSessionSystemPassword() const
{
    return m_sessionSystemPassword;
}

unsigned short NexJoinSessionSetting::GetExtraParticipantNum() const
{
    return m_extraParticipantNum;
}

unsigned int NexJoinSessionSetting::GetMatchmakeSessionOption() const
{
    return m_matchmakeSessionOption;
}

bool NexJoinSessionSetting::IsSetMatchmakeSessionOption() const
{
    return m_isSetMatchmakeSessionOption;
}

}}}
