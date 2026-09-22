#include "nn/pia/inet/NexMatchClearSystemPasswordJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchClearSystemPasswordJob::NexMatchClearSystemPasswordJob() : m_result(0), m_started(false)
{}

NexMatchClearSystemPasswordJob::~NexMatchClearSystemPasswordJob()
{}

void NexMatchClearSystemPasswordJob::Cleanup()
{
    m_result = 0; m_started = false;

}
unsigned int NexMatchClearSystemPasswordJob::ClearMatchmakeSystemPassword()
{
    m_started = true; return m_result;

}
unsigned int NexMatchClearSystemPasswordJob::StartupImpl()
{
    m_started = true; return m_result;

}
void NexMatchClearSystemPasswordJob::Trace(unsigned long long p0)
{
    (void)p0;

}
unsigned int NexMatchClearSystemPasswordJob::WaitClearMatchmakeSystemPassword()
{
    return m_started ? m_result : 0;

}

}}}
