#include "nn/pia/common/Job.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace common {

Job::Job()
    : m_innerState(cInnerStateNotEntried), m_executeCount(0), m_time()
{
    m_listNode.prev = 0;
    m_listNode.next = 0;
}

Job::~Job()
{
}

void Job::Reset(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    if (m_innerState != cInnerStateNotEntried &&
        m_innerState < cInnerStateForegroundRunning)
    {
        g_piaJobScheduler->ResetJob(this);
    }
    m_innerState = cInnerStateNotEntried;
    m_executeCount = 0;
    m_time = Time();
    criticalSection.Unlock();
}

void Job::Ready(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    if (m_innerState == cInnerStateNotEntried)
    {
        m_innerState = isBackground ? cInnerStateBackgroundReady
                                    : cInnerStateForegroundReady;
        m_time.SetNow();
        g_piaJobScheduler->EntryJob(this, isBackground);
    }
    criticalSection.Unlock();
}

void Job::Resume(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    if (m_innerState == cInnerStateSuspended)
    {
        m_innerState = isBackground ? cInnerStateBackgroundReady
                                    : cInnerStateForegroundReady;
        m_time.SetNow();
        g_piaJobScheduler->EntryJob(this, isBackground);
    }
    criticalSection.Unlock();
}

Job::State Job::GetState() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    State state;
    switch (m_innerState)
    {
    case cInnerStateForegroundWaiting:
    case cInnerStateBackgroundWaiting:
        state = cStateWaiting;
        break;
    case cInnerStateForegroundReady:
    case cInnerStateBackgroundReady:
        state = cStateReady;
        break;
    case cInnerStateForegroundRunning:
    case cInnerStateBackgroundRunning:
        state = cStateRunning;
        break;
    case cInnerStateSuspended:
        state = cStateSuspended;
        break;
    case cInnerStateCompleted:
        state = cStateCompleted;
        break;
    default:
        state = cStateNotEntried;
        break;
    }
    criticalSection.Unlock();
    return state;
}

void Job::Execute(bool isBackground)
{
    m_innerState = isBackground ? cInnerStateBackgroundRunning
                                : cInnerStateForegroundRunning;
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Unlock();
    const unsigned int result = ExecuteCore();
    criticalSection.Lock();
    m_executeCount = m_executeCount + 1;

    switch (result & 0xff000000u)
    {
    case cExecuteResultFinished:
    case cExecuteResultCancelled:
        m_innerState = cInnerStateCompleted;
        return;
    case cExecuteResultSuspend:
        m_innerState = cInnerStateSuspended;
        return;
    case cExecuteResultWait:
        m_time.SetNow();
        m_time.m_tick += TimeSpan::GetTicksPerMSec().m_tick * (result & 0xffff);
        g_piaJobScheduler->EntryJob(this, isBackground);
        m_innerState = isBackground ? cInnerStateBackgroundWaiting
                                    : cInnerStateForegroundWaiting;
        return;
    case cExecuteResultNextStep:
        m_time.SetNow();
        g_piaJobScheduler->EntryJobNext(this);
        m_innerState = isBackground ? cInnerStateBackgroundWaiting
                                    : cInnerStateForegroundWaiting;
        return;
    case cExecuteResultContinue:
    case cExecuteResultEntryForeground:
    case cExecuteResultEntryBackground:
        break;
    default:
        return;
    }

    m_time.SetNow();
    if ((result & 0xff000000u) == cExecuteResultEntryForeground)
    {
        g_piaJobScheduler->EntryJob(this, false);
        m_innerState = cInnerStateForegroundReady;
        return;
    }
    if ((result & 0xff000000u) == cExecuteResultEntryBackground)
    {
        g_piaJobScheduler->EntryJob(this, true);
        m_innerState = cInnerStateBackgroundReady;
        return;
    }
    g_piaJobScheduler->EntryJob(this, isBackground);
    m_innerState = isBackground ? cInnerStateBackgroundReady
                                : cInnerStateForegroundReady;
}

bool Job::IsRunning() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    const bool running = m_innerState != cInnerStateNotEntried &&
                         m_innerState < cInnerStateCompleted;
    criticalSection.Unlock();
    return running;
}

bool Job::IsForeground() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    const bool foreground = m_innerState != cInnerStateNotEntried &&
                            m_innerState <= cInnerStateForegroundRunning;
    criticalSection.Unlock();
    return foreground;
}

bool Job::IsBackground() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    const bool background = m_innerState > cInnerStateForegroundRunning &&
                            m_innerState <= cInnerStateBackgroundRunning;
    criticalSection.Unlock();
    return background;
}

}}}
