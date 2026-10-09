#include "nn/nex/RoutingStream.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/StreamSettings.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ROUTING_HEADER_SIZE = 6;
const unsigned int ROUTING_CRITICAL_SECTION_TRACE_MASK = 1;
const unsigned int ROUTING_TABLE_TIMEOUT_FACTOR = 3;

unsigned int s_uiHistoryCleaningPeriod = 0;
unsigned int s_uiHistoryLifetime = 10000;

}

RoutingStream::RoutingStream()
    : Stream(Routing)
    , m_criticalSection(ROUTING_CRITICAL_SECTION_TRACE_MASK)
    , m_routingTable(g_streamSettings[Unknown1].Get().GetMaxSilenceTime()
          * ROUTING_TABLE_TIMEOUT_FACTOR)
    , m_pTransport(0)
    , m_pFilter(0)
{
    m_lastCleaning = Time(0);
}

RoutingStream::~RoutingStream()
{
}

PacketOut* RoutingStream::BuildRoutingPacket(Buffer* pPayload, PacketType type,
    const InetAddress& rAddress)
{
    Buffer* pBuffer;
    if (pPayload == 0 || !pPayload->IsPooled())
    {
        pBuffer = new Buffer(Buffer::s_uiDefaultBufferSize, 0);
    }
    else
    {
        pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(pPayload->GetLength() + ROUTING_HEADER_SIZE, Buffer::BUFFER_FLAG_POOLED);
        if (pBuffer == 0)
            return 0;
        if (!pBuffer->IsValid())
        {
            delete pBuffer;
            return 0;
        }
    }
    ByteStream stream(pBuffer);
    pBuffer->ReleaseRef();
    unsigned int uiAddress = rAddress.GetAddress();
    stream.Append(reinterpret_cast<const unsigned char*>(&uiAddress), 4, 1);
    unsigned short uiPort = rAddress.GetPortNumber();
    stream.Append(reinterpret_cast<const unsigned char*>(&uiPort), 2, 1);
    stream << *pPayload;
    PacketOut* pPacket;
    if (pPayload != 0 && pPayload->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(0, type, 0, stream.GetBuffer(), 0);
        if (pPacket == 0 || !pPacket->IsValid())
        {
            delete pPacket;
            return 0;
        }
    }
    else
    {
        pPacket = new PacketOut(0, type, 0, stream.GetBuffer(), 0);
    }
    pPacket->m_uiVersion = 0;
    return pPacket;
}

void RoutingStream::DoWork()
{
    if (s_uiHistoryCleaningPeriod != 0)
    {
        Time now(Time::GetTime());
        if (static_cast<long long>(now - m_lastCleaning) >
            static_cast<long long>(s_uiHistoryCleaningPeriod))
        {
            m_lastCleaning = now;
            CleanHostRoutingHistory();
        }
    }
}

void RoutingStream::CleanHostRoutingHistory()
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    Time now(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_history.erase(it++);
        else
            ++it;
    }

    for (DenyMap::iterator it = m_denied.begin(); it != m_denied.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_denied.erase(it++);
        else
            ++it;
    }

    rCriticalSection.Leave();
}

unsigned int RoutingStream::GetHistoryCount(const InetAddress* pAddress)
{
    if (pAddress == 0)
        return m_history.size();

    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    unsigned int uiCount = 0;
    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end(); ++it)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            ++uiCount;
    }
    rCriticalSection.Leave();
    return uiCount;
}

void RoutingStream::SetDenyForAddress(const InetAddress* pAddress)
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    m_denied[*pAddress] = Time(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            m_history.erase(it++);
        else
            ++it;
    }
    rCriticalSection.Leave();
}

}
}
