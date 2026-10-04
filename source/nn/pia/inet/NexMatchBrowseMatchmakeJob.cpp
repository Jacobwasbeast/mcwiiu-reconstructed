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
Result NexMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession, const session::SessionSearchCriteria* searchCriteria)
{
    m_state = 0;
    return Result(0);

}
Result NexMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession, unsigned int sessionId)
{
    return Result(m_state);

}
void NexMatchBrowseMatchmakeJob::Trace(unsigned long long p0) const
{

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
