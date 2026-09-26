#include "nn/pia/common/StepSequenceJob.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

namespace nn { namespace pia { namespace common {

StepSequenceJob::Step::~Step()
{
}

void StepSequenceJob::Step::Trace(unsigned long long value) const
{
}

StepSequenceJob::StepSequenceJob()
    : Job(), m_currentStep(), m_cancelRequested(false), m_state(0), m_result(0)
{
}

StepSequenceJob::~StepSequenceJob()
{
}

void StepSequenceJob::Reset(bool isBackground)
{
    Job::Reset(isBackground);
    m_cancelRequested = false;
}

unsigned int StepSequenceJob::ExecuteCore()
{
    for (;;)
    {
        if (m_cancelRequested)
        {
            CancelCleanup();
            m_cancelRequested = false;
            return cExecuteResultCancelled;
        }
        const unsigned int result = (this->*m_currentStep.m_function)();
        if ((result & 0xff000000u) != cExecuteResultContinue)
            return result;
    }
}

void StepSequenceJob::WaitForCompletion(unsigned int intervalMsec)
{
    while (IsRunning())
        Sleep(intervalMsec);
}

void StepSequenceJob::CancelCleanup()
{

}

void StepSequenceJob::Trace(unsigned long long value) const
{
}

}}}
