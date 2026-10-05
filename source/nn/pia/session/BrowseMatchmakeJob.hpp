#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_BROWSE_MATCHMAKE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_BROWSE_MATCHMAKE_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class IMatchmakeSession;
class SessionSearchCriteria;

class BrowseMatchmakeJob : public common::StepSequenceJob
{
public:
    BrowseMatchmakeJob();
    virtual ~BrowseMatchmakeJob();

    Result Startup(common::CallContext* callerContext,
                   IMatchmakeSession* matchmakeSession,
                   const SessionSearchCriteria* criteria);
    Result Startup(common::CallContext* callerContext,
                   IMatchmakeSession* matchmakeSession,
                   unsigned int sessionId);
    ExecuteResult CompleteProcess();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual Result StartupImpl(IMatchmakeSession* matchmakeSession,
                               const SessionSearchCriteria* criteria) = 0;
    virtual Result StartupImpl(IMatchmakeSession* matchmakeSession,
                               unsigned int sessionId) = 0;

    common::CallContext* m_callContext;
};

}}}

#endif
