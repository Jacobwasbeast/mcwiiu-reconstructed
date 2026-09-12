#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/session/SessionInfo.hpp"

namespace nn { namespace pia { namespace session {

class JoinSessionSetting
{
public:
    JoinSessionSetting();
    virtual ~JoinSessionSetting();

    virtual void Reset();
    virtual void Trace(unsigned long long value) const;

    const ISessionInfo* GetSessionInfoPtr() const;
    void SetSessionInfoPtr(const ISessionInfo* sessionInfo);
    void SetSessionId(unsigned int sessionId);
    unsigned int GetSessionId() const;
    void SetMaxParticipantNum(unsigned short participantNum);
    unsigned int GetMaxParticipantNum() const;

    const ISessionInfo* m_sessionInfo;
    unsigned int m_sessionId;
    unsigned short m_maxParticipantNum;
};

}}}

#endif
