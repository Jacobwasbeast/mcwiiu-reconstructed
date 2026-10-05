#include "nn/nex/PacketDispatchQueue.hpp"

#include "nn/nex/PacketIn.hpp"

namespace nn
{
namespace nex
{

PacketDispatchQueue::PacketDispatchQueue()
    : m_packets()
    , m_nextToDispatch()
{
    m_nextToDispatch = 1;
}

PacketDispatchQueue::~PacketDispatchQueue()
{
    Purge();
}

void PacketDispatchQueue::Queue(PacketIn* pPacket)
{
    Clock sequence = pPacket->m_sequenceID;
    if (sequence == m_nextToDispatch || sequence - m_nextToDispatch > 0)
    {
        if (m_packets.find(sequence) == m_packets.end())
        {
            pPacket->AddRef();
            m_packets[sequence] = pPacket;
        }
    }
}

PacketIn* PacketDispatchQueue::GetNextToDispatch()
{
    PacketIn* pPacket = 0;
    PacketMap::iterator found = m_packets.find(m_nextToDispatch);
    if (found != m_packets.end())
    {
        pPacket = found->second;
        ++m_nextToDispatch;
    }
    return pPacket;
}

void PacketDispatchQueue::Dispatched(PacketIn* pPacket)
{
    m_packets.erase(pPacket->m_sequenceID);
    pPacket->ReleaseRef();
}

void PacketDispatchQueue::Purge()
{
    PacketMap::iterator it = m_packets.begin();
    while (it != m_packets.end())
    {
        it->second->ReleaseRef();
        PacketMap::iterator current = it;
        ++it;
        m_packets.erase(current);
    }
}

}
}
