#include "nn/nex/StepSequenceJob.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Callback.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

StepSequenceJob::Step::Step()
    : m_function(0)
    , m_pActivity(0)
{
}

StepSequenceJob::Step::Step(Function function, const wchar_t* pActivity)
    : m_function(function)
    , m_pActivity(pActivity)
{
}

StepSequenceJob::Step::~Step()
{
}

StepSequenceJob::StepSequenceJob(JobType jobType, const DebugString& rDebugString)
    : Job(jobType, rDebugString)
    , m_stepStartTime(0)
    , m_executionCount(0)
    , m_step()
    , m_traceData(0)
{
}

StepSequenceJob::~StepSequenceJob()
{
}

void StepSequenceJob::SetStep(const Step& rStep)
{
    m_stepStartTime = Time::GetTime();
    m_step = rStep;
}

CallbackRoot* StepSequenceJob::CreateResumeCompletionCallback(Step* pStep)
{
    AddRef();
    return new Callback<StepSequenceJob, Step*>(this, &StepSequenceJob::ProcessCallResult, pStep);
}

void StepSequenceJob::ResumeOnCallCompletion(CallContext* pContext, Step* pStep)
{
    CallbackRoot* pCallback = CreateResumeCompletionCallback(pStep);
    pContext->RegisterCompletionCallback(pCallback, false, true);
}

void StepSequenceJob::ResumeOnCallCompletionOrAlreadyDone(CallContext* pContext, Step* pStep)
{
    if (pContext->GetState() == CallContext::CallInProgress)
    {
        ResumeOnCallCompletion(pContext, pStep);
        return;
    }

    SetToReady();
    SetStep(*pStep);
    delete pStep;
}

String StepSequenceJob::GetTraceInfo() const
{
    return String("");
}

void StepSequenceJob::Execute()
{
    for (;;)
    {
        CheckExceptions();
        if (GetState() != Running)
            return;

        std::uint32_t contextId = 0;
        if (g_usingInstantiationContext)
            contextId = g_instantiationContextVariable->GetRef();

        if (contextId != 0)
        {
            const InstanceTable::ContextVector* contexts = g_instanceTable.m_extraContexts;
            if (contexts->size() <= contextId)
            {
                SystemError::SignalError(
                    L"D:\\home\\jenkins\\workspace\\Master\\Pack\\d4f027b8\\.\\OnlineCore/src/Core/InstanceTable.h",
                    0x48, 0xE0000003, 0);
            }
        }

        (this->*m_step.m_function)();
        ++m_executionCount;

        if (GetState() != Running)
            return;
    }
}

void StepSequenceJob::ProcessCallResult(Step* pStep)
{
    SetToReady();
    SetStep(*pStep);
    delete pStep;
    ReleaseRef();
}

void StepSequenceJob::AddActivity(const wchar_t*)
{
}

void StepSequenceJob::CheckExceptions()
{
}

}
}
