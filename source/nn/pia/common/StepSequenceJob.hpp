#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STEP_SEQUENCE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STEP_SEQUENCE_JOB_HPP

#include "nn/pia/common/Job.hpp"

namespace nn { namespace pia { namespace common {

class StepSequenceJob : public Job
{
public:
    class Step
    {
    public:
        Step() : m_function(0), m_name(0) {}

        virtual ~Step();
        virtual void Trace(unsigned long long value) const;

        unsigned int (StepSequenceJob::*m_function)();
        const char* m_name;
    };

    StepSequenceJob();
    virtual ~StepSequenceJob();
    virtual void Reset(bool isBackground);
    virtual unsigned int ExecuteCore();
    virtual void CancelCleanup();
    virtual void Trace(unsigned long long value) const;
    void WaitForCompletion(unsigned int intervalMsec);

    Step m_currentStep;
    bool m_cancelRequested;
    unsigned int m_state;
    unsigned int m_result;
};

}}}

#endif
