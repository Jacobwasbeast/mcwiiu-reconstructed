#include "nn/nex/TransportStreamManager.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

TransportStreamManager::TransportStreamManager()
{
    m_streams.reserve(16);
}

TransportStreamManager::~TransportStreamManager()
{
    while (!m_streams.empty())
    {
        VirtualPort port = m_streams.front().first;
        UnregisterStream(port.GetType(), port.GetStreamID());
    }
}

int TransportStreamManager::GetNbStream()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return static_cast<int>(m_streams.size());
}

Stream* TransportStreamManager::GetStreamImpl(VirtualPort port)
{
    qSortedVector<VirtualPort, Stream*>::iterator position = m_streams.find(port);
    if (position == m_streams.end())
        return 0;

    return position->second;
}

Stream* TransportStreamManager::GetStream(VirtualPort port)
{
    if (!g_transportThreadEnabled)
        return GetStreamImpl(port);

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return GetStreamImpl(port);
}

bool TransportStreamManager::UnregisterStream(Stream::Type type, unsigned char uiStreamID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    VirtualPort port(type, uiStreamID);
    if (GetStream(port) == 0)
        return false;

    m_streams.erase(port);
    return true;
}

}
}
