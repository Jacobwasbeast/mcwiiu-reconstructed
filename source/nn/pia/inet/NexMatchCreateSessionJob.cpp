#include "nn/pia/inet/NexMatchCreateSessionJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchCreateSessionJob::NexMatchCreateSessionJob() : m_state(0)
{}

NexMatchCreateSessionJob::~NexMatchCreateSessionJob()
{}

void NexMatchCreateSessionJob::Cleanup()
{
    m_state = 0;

}
unsigned int NexMatchCreateSessionJob::CreateMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::LeaveJoinedMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::SetNextStepAtFailedCreateMesh(Result result)
{
    return m_state;

}
void NexMatchCreateSessionJob::SetNextStepAtWaitCreateMesh()
{
    m_state = 0;

}
unsigned int NexMatchCreateSessionJob::StartNatSession()
{
    return m_state;

}
Result NexMatchCreateSessionJob::StartupImpl(const session::CreateSessionSetting* setting)
{
    return Result(m_state);

}
void NexMatchCreateSessionJob::Trace(unsigned long long p0) const
{

}
unsigned int NexMatchCreateSessionJob::UnregisterGathering()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::WaitCreateMatchmake()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::WaitLeaveJoinedMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::WaitNotification()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::WaitStartNatSession()
{
    return m_state;

}
unsigned int NexMatchCreateSessionJob::WaitUnregisterGathering()
{
    return m_state;

}

}}}
