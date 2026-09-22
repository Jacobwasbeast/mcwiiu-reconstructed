#include "nn/pia/inet/NexMatchBrowseMatchmakeJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchBrowseMatchmakeJob::NexMatchBrowseMatchmakeJob() : m_state(0)
{}

NexMatchBrowseMatchmakeJob::~NexMatchBrowseMatchmakeJob()
{}

unsigned int NexMatchBrowseMatchmakeJob::BrowseMatchmake()
{
    return m_state;

}
void NexMatchBrowseMatchmakeJob::Cleanup()
{
    m_state = 0;

}
void NexMatchBrowseMatchmakeJob::StartupImpl(unsigned long long p0, unsigned long long p1)
{
    m_state = 0;

}
unsigned long long NexMatchBrowseMatchmakeJob::StartupImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2)
{
    return m_state;

}
void NexMatchBrowseMatchmakeJob::Trace(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchBrowseMatchmakeJob::WaitBrowseMatchmake()
{
    return m_state;

}
unsigned int NexMatchBrowseMatchmakeJob::WaitFindSessionByOwner()
{
    return m_state;

}
unsigned int NexMatchBrowseMatchmakeJob::WaitFindSessionByParticipant()
{
    return m_state;

}
unsigned int NexMatchBrowseMatchmakeJob::WaitFindSessionBySessionId()
{
    return m_state;

}
unsigned int NexMatchBrowseMatchmakeJob::WaitRequestSessionInfo()
{
    return m_state;

}

}}}
