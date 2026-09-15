#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace session {

class JoinSessionSetting;

class JoinSessionJob : public common::StepSequenceJob
{
public:
    JoinSessionJob();
    virtual ~JoinSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    virtual void CleanupInternalContexts();
    virtual bool CheckContextCallCancelled();
    virtual void Cleanup();
    virtual unsigned int GetJoinSessionStateForDebug() const;

    Result Startup(common::CallContext* callerContext, const JoinSessionSetting* setting);

protected:
    virtual Result StartupImpl(const JoinSessionSetting* setting) = 0;
    virtual void CleanupImpl();

public:

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    transport::StationConnectionInfo m_stationConnectionInfo;
    unsigned int m_joinState;
    unsigned int m_sessionId;
    bool m_joinOption;
    bool m_connectionInfoRetry;
    bool m_sessionInfoRetry;
    bool m_retryReasonB;
    bool m_sessionInvalidated;
    bool m_ownerChanged;
    bool m_retryReasonA;
    unsigned char m_retryCount;
    Result m_result;
    common::Time m_startTime;
};

}}}

#endif
