#include "nn/pia/inet/NexMatchDestroySessionJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchDestroySessionJob::NexMatchDestroySessionJob() : m_state(0)
{}

NexMatchDestroySessionJob::~NexMatchDestroySessionJob()
{}

void NexMatchDestroySessionJob::Cleanup()
{
    m_state = 0;

}
void NexMatchDestroySessionJob::SetNextStepAtMeshCleanup()
{
    m_state = 0;

}
void NexMatchDestroySessionJob::SetNextStepAtWaitDestroyMesh()
{
    m_state = 0;

}
unsigned int NexMatchDestroySessionJob::StartupImpl()
{
    return m_state;

}
void NexMatchDestroySessionJob::Trace(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchDestroySessionJob::UnregisterBufferMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchDestroySessionJob::WaitUnregisterBufferMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession()
{
    return m_state;

}

}}}
