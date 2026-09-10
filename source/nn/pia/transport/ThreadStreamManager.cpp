#include "nn/pia/transport/ThreadStreamManager.hpp"

namespace nn { namespace pia { namespace transport {

ThreadStreamManager* g_piaThreadStreamManager = 0;

Result ThreadStreamManager::Startup()
{
    if (m_receiveThreadStream == 0 || m_sendThreadStream == 0)
        return static_cast<Result>(0x10c08);

    m_receiveThreadStream->TransportThreadStream::Startup();
    m_sendThreadStream->TransportThreadStream::Startup();
    return 0;
}

void ThreadStreamManager::Cleanup()
{
    if (m_receiveThreadStream != 0)
        m_receiveThreadStream->TransportThreadStream::Cleanup();
    if (m_sendThreadStream != 0)
        m_sendThreadStream->TransportThreadStream::Cleanup();
}

void ThreadStreamManager::SetSendThreadPriority(int priority)
{
    m_sendThreadStream->m_thread->setPriority(priority);
}

int ThreadStreamManager::GetSendThreadPriority() const
{
    return m_sendThreadStream->m_thread->getPriority();
}

void ThreadStreamManager::SetReceiveThreadPriority(int priority)
{
    m_receiveThreadStream->m_thread->setPriority(priority);
}

int ThreadStreamManager::GetReceiveThreadPriority() const
{
    return m_receiveThreadStream->m_thread->getPriority();
}

}}}
