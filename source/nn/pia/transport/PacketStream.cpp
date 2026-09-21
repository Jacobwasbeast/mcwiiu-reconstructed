#include "nn/pia/transport/PacketStream.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/HeapManager.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

common::Packet* PacketStream::Accessor::Get(int index)
{
    unsigned int packetIndex = static_cast<unsigned int>(index) +
        static_cast<unsigned int>(m_field04);
    if (static_cast<int>(packetIndex) >= static_cast<int>(m_packetStream->m_packetNum))
        packetIndex -= m_packetStream->m_packetNum;
    if (packetIndex >= m_packetStream->m_packetNum)
        return 0;
    return m_packetStream->m_packets + packetIndex;
}

int PacketStream::Writer::AssignableNum() const
{
    const int readerPosition = m_packetStream->m_reader.m_field04;
    const int available = readerPosition - m_field08 - 1;
    if (m_field08 < readerPosition)
        return available;
    return available + static_cast<int>(m_packetStream->m_packetNum);
}

common::Packet* PacketStream::Writer::Assign()
{
    if (AssignableNum() < 1)
        return 0;

    const unsigned int packetIndex = static_cast<unsigned int>(m_field08);
    int nextPosition = m_field08 + 1;
    if (nextPosition >= static_cast<int>(m_packetStream->m_packetNum))
        nextPosition = 0;
    m_field08 = nextPosition;
    ++m_field0c;

    const int packetNum = static_cast<int>(m_packetStream->m_packetNum);
    const int usedCount = packetNum - AssignableNum();
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(m_packetStream->m_field28)->Update(usedCount);

    if (packetIndex < m_packetStream->m_packetNum)
        return &m_packetStream->m_packets[packetIndex];
    return 0;
}

void PacketStream::Writer::Push()
{
    m_field04 = m_field08;
    m_field0c = 0;
}

void PacketStream::Reader::PullAll()
{
    m_field08 = m_packetStream->m_writer.m_field04;
    const int count = m_field08 - m_field04;
    m_field0c = count;
    if (count < 0)
        m_field0c += m_packetStream->m_packetNum;
}

void PacketStream::Reader::Release()
{
    m_field04 = m_field08;
    m_field0c = 0;
}

PacketStream::PacketStream()
    : m_packetNum(0), m_packets(0), m_writer(this), m_reader(this)
{
}

Result PacketStream::Initialize(unsigned int packetNum, unsigned int field28)
{
    if (packetNum < 2)
        return static_cast<Result>(0x10c07);
    if (m_packets != 0)
        return static_cast<Result>(0x10c02);

    pead::Heap* heap = common::HeapManager::GetHeap();
    if (static_cast<int>(packetNum) > 0)
    {
        if (heap == 0)
            heap = pead::HeapMgr::instance()->getCurrentHeap();
        common::Packet* packets = static_cast<common::Packet*>(
            heap->tryAlloc(packetNum * sizeof(common::Packet), 4));
        if (packets != 0)
        {
            for (unsigned int index = 0; index < packetNum; ++index)
                new (packets + index) common::Packet();
            m_packets = packets;
            m_packetNum = packetNum;
        }
    }
    m_field28 = field28;
    Cleanup();
    return 0;
}

void PacketStream::Finalize()
{
    if (m_packets != 0)
    {
        for (int index = 0; index < static_cast<int>(m_packetNum); ++index)
            m_packets[index].~Packet();
        pead::HeapMgr::instance()->findContainHeap(m_packets)->free(m_packets);
        m_packetNum = 0;
        m_packets = 0;
    }
    Cleanup();
}

Result PacketStream::Startup()
{
    if (m_packets == 0)
        return static_cast<Result>(0x10c0c);

    m_writer.m_field04 = 0;
    m_writer.m_field08 = 0;
    m_writer.m_field0c = 0;
    m_reader.m_field04 = 0;
    m_reader.m_field08 = 0;
    m_reader.m_field0c = 0;
    return 0;
}

void PacketStream::Cleanup()
{
    m_writer.m_field04 = -1;
    m_writer.m_field08 = -1;
    m_writer.m_field0c = 0;
    m_reader.m_field04 = -1;
    m_reader.m_field08 = -1;
    m_reader.m_field0c = 0;
}

void PacketStream::Trace(unsigned long long value) const
{
}

}}}
