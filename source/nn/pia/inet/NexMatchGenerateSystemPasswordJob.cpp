#include "nn/pia/inet/NexMatchGenerateSystemPasswordJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchGenerateSystemPasswordJob::NexMatchGenerateSystemPasswordJob() : m_result(0), m_started(false)
{}

NexMatchGenerateSystemPasswordJob::~NexMatchGenerateSystemPasswordJob()
{}

void NexMatchGenerateSystemPasswordJob::Cleanup()
{
    m_result = 0; m_started = false;

}
unsigned int NexMatchGenerateSystemPasswordJob::GenerateMatchmakeSystemPassword()
{
    m_started = true; return m_result;

}
unsigned int NexMatchGenerateSystemPasswordJob::StartupImpl()
{
    m_started = true; return m_result;

}
void NexMatchGenerateSystemPasswordJob::Trace(unsigned long long p0)
{
    (void)p0;

}
unsigned int NexMatchGenerateSystemPasswordJob::WaitGenerateMatchmakeSystemPassword()
{
    return m_started ? m_result : 0;

}

}}}
