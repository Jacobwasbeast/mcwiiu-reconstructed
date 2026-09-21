#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STEPSEQUENCEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STEPSEQUENCEJOB_HPP

#include <cstdint>

#include "nn/nex/Job.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class DebugString;
class SystemComponent;
class CallbackRoot;

class StepSequenceJob : public Job
{
public:
    class Step : public RootObject
    {
    public:
        typedef void (StepSequenceJob::*Function)();

        Step();
        Step(Function function, const wchar_t* pActivity);
        virtual ~Step();

    private:
        friend class StepSequenceJob;

        Function m_function;
        const wchar_t* m_pActivity;
    };

    StepSequenceJob(JobType jobType, const DebugString& rDebugString);
    virtual ~StepSequenceJob();

    virtual void Execute();
    virtual void AddActivity(const wchar_t* pActivity);
    virtual String GetTraceInfo() const;
    virtual void CheckExceptions();

    void SetStep(const Step& rStep);
    CallbackRoot* CreateResumeCompletionCallback(Step* pStep);
    void ResumeOnCallCompletion(CallContext* pContext, Step* pStep);
    void ResumeOnCallCompletionOrAlreadyDone(CallContext* pContext, Step* pStep);
    void ProcessCallResult(Step* pStep);

    void TerminateComponent(SystemComponent* pComponent, const Step& rStep);
    void AddStepActivity();

private:
    std::uint64_t m_stepStartTime;
    std::uint32_t m_executionCount;
    Step m_step;

protected:
    std::uint64_t m_traceData;
};

}
}

#endif
