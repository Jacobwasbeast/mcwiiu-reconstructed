#include "nn/pia/session/SessionStatusCheckJob.hpp"

namespace nn { namespace pia { namespace session {

SessionStatusCheckJob::SessionStatusCheckJob()
    : common::StepSequenceJob(), m_callContext(), m_field54(false), m_field55(false)
{
}

SessionStatusCheckJob::~SessionStatusCheckJob()
{
}

void SessionStatusCheckJob::Trace(unsigned long long value) const
{
}

void SessionStatusCheckJob::Cleanup()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_field54 = false;
    CancelCleanup();
    Reset(true);
}

}}}
