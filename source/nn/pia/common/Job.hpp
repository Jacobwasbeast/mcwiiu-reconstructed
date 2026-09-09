#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn { namespace pia { namespace common {

class Job
{
public:
    enum InnerState
    {
        cInnerStateNotEntried = 0,
        cInnerStateForegroundWaiting = 1,
        cInnerStateForegroundReady = 2,
        cInnerStateForegroundRunning = 3,
        cInnerStateBackgroundWaiting = 4,
        cInnerStateBackgroundReady = 5,
        cInnerStateBackgroundRunning = 6,
        cInnerStateSuspended = 7,
        cInnerStateCompleted = 8
    };

    enum State
    {
        cStateNotEntried = 0,
        cStateWaiting = 1,
        cStateSuspended = 2,
        cStateReady = 3,
        cStateRunning = 4,
        cStateCompleted = 5
    };

    enum ExecuteResult
    {
        cExecuteResultContinue = 0x00000000,
        cExecuteResultFinished = 0x01000000,
        cExecuteResultCancelled = 0x02000000,
        cExecuteResultSuspend = 0x03000000,
        cExecuteResultWait = 0x04000000,
        cExecuteResultNextStep = 0x05000000,
        cExecuteResultEntryForeground = 0x06000000,
        cExecuteResultEntryBackground = 0x07000000
    };

    Job();
    virtual ~Job();
    virtual void Reset(bool isBackground);
    virtual unsigned int ExecuteCore() = 0;
    void Ready(bool isBackground);
    void Resume(bool isBackground);
    State GetState() const;
    bool IsRunning() const;
    bool IsForeground() const;
    bool IsBackground() const;
    void Execute(bool isBackground);
    static const char* GetInnerStateString(InnerState innerState);

    InnerState m_innerState;
    unsigned int m_executeCount;
    Time m_time;
    ListNode m_listNode;
};

}}}

#endif
