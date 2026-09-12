#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class JoinSessionSetting;

class JoinSessionJob : public common::StepSequenceJob
{
public:
    JoinSessionJob();
    virtual ~JoinSessionJob();
    virtual void Trace(unsigned long long value) const;

    Result Startup(common::CallContext* callerContext, const JoinSessionSetting* setting);
    void Cleanup();

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
};

}}}

#endif
