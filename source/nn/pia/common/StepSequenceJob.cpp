#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace common {

StepSequenceJob::Step::~Step()
{
}

void StepSequenceJob::Step::Trace(unsigned long long value) const
{
}

StepSequenceJob::StepSequenceJob()
    : Job(), m_currentStep(), m_cancelRequested(false), m_field38(0), m_field3c(0)
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

void StepSequenceJob::CancelCleanup()
{
}

void StepSequenceJob::Trace(unsigned long long value) const
{
}

}}}
