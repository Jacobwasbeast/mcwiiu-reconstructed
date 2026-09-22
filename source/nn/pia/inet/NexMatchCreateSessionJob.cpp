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
unsigned int NexMatchCreateSessionJob::SetNextStepAtFailedCreateMesh(unsigned long long p0, unsigned long long p1, unsigned long long p2)
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
unsigned long long NexMatchCreateSessionJob::StartupImpl(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchCreateSessionJob::Trace(unsigned long long p0)
{
    m_state = 0;

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
