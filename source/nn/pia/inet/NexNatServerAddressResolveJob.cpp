#include "nn/pia/inet/NexNatServerAddressResolveJob.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/nat/Api.hpp"

namespace nn { namespace pia { namespace inet {

const nat::DebugSetting* GetDebugSetting();

namespace {
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNatServerAddressUnavailable = 0x6436
};

typedef common::Job::ExecuteResult (NexNatServerAddressResolveJob::*Step)();

inline void SelectStep(common::StepSequenceJob::Step& slot, Step step)
{
    union { Step derived; common::Job::ExecuteResult (common::StepSequenceJob::*base)(); } value;
    value.derived = step;
    slot.m_function = value.base;
}

inline void SetDeadline(common::Time& deadline, unsigned int milliseconds)
{
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                      + common::TimeSpan::GetTicksPerMSec().m_tick * milliseconds;
}
}

NexNatServerAddressResolveJob::NexNatServerAddressResolveJob()
    : common::StepSequenceJob(), m_callContext(0), m_deadline(), m_result(0)
{
}

NexNatServerAddressResolveJob::~NexNatServerAddressResolveJob()
{
}

Result NexNatServerAddressResolveJob::Startup(common::CallContext* callContext)
{
    if (callContext == 0)
        return cResultInvalidArgument;
    if (!callContext->InitiateCall())
        return cResultInvalidState;
    m_callContext = callContext;
    m_deadline = common::g_piaJobScheduler->m_dispatchTime;
    m_result = 0;
    m_currentStep.m_name = "NexNatServerAddressResolveJob::StepResolveNatServerAddress";
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepResolveNatServerAddress);
    common::StepSequenceJob::Reset(true);
    return 0;
}

void NexNatServerAddressResolveJob::Cleanup()
{
    m_callContext = 0;
}

void NexNatServerAddressResolveJob::CancelCleanup()
{
    if (m_callContext != 0 && m_callContext->m_state == common::CallContext::cStateInProgress)
        m_callContext->SignalCancel();
    Cleanup();
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepResolveNatServerAddress()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    if (GetDebugSetting()->options[3] != 0)
    {
        m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
        m_result = cResultNatServerAddressUnavailable;
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    SetDeadline(m_deadline, 30000);
    m_result = NexFacade::s_instance->UpdateNatServerAddress();
    if (m_result != 0)
    {
        m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    m_currentStep.m_name = "NexNatServerAddressResolveJob::StepRequestRelayServerSetting";
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepRequestRelayServerSetting);
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepRequestRelayServerSetting()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SetDeadline(m_deadline, 10000);
    NexFacade::s_instance->m_natRelay->RequestRelayServerSettingAsync();
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting);
    m_currentStep.m_name = "NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting";
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting()
{
    NexFacade* facade = NexFacade::s_instance;
    if (facade->IsRelayServiceEnabled() && common::g_piaJobScheduler->m_dispatchTime <= m_deadline)
    {
        if (!facade->m_natRelay->IsRequestRelayServerSettingCompleted())
            return MakeExecuteResult(cExecuteKindNextStep);
        const Result result = facade->m_natRelay->GetRequestRelayServerSettingResult();
        if (result == 0 && m_callContext != 0 && !m_callContext->m_cancelRequested)
        {
            SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepResolveRelayServerSetting);
            m_currentStep.m_name = "NexNatServerAddressResolveJob::StepResolveRelayServerSetting";
            return MakeExecuteResult(cExecuteKindEntryBackground);
        }
    }
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
    m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepResolveRelayServerSetting()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
        m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    if (NexFacade::s_instance->IsRelayServiceEnabled())
    {
        NexFacade::s_instance->m_natRelay->ResolveRelayServerSetting();
        NexFacade::s_instance->m_natRelay->IsRelayServerAvailable();
    }
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete);
    m_currentStep.m_name = "NexNatServerAddressResolveJob::StepComplete";
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepComplete()
{
    if (m_callContext != 0)
    {
        if (m_callContext->m_cancelRequested)
        {
            m_callContext->SignalCancel();
        }
        else if (m_result == 0)
        {
            m_callContext->SignalSuccess(0);
        }
        else
        {
            m_callContext->SignalFailure(m_result);
        }
    }
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

}}}
