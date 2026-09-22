#include "nn/pia/transport/LatencyEmulator.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include <new>

namespace nn { namespace pia { namespace transport {

bool LatencyEmulator::allocateBuffer(unsigned int entryNum)
{
    if (entryNum == 0)
        return false;

    m_buffer = static_cast<Entry*>(pead::allocateFromSeadHeap(
        entryNum * sizeof(Entry), common::HeapManager::GetHeap()));
    if (m_buffer == 0)
    {
        m_entries = 0;
        return false;
    }

    for (unsigned int index = 0; index < entryNum; ++index)
    {
        new (&m_buffer[index]) Entry();
        m_buffer[index].m_dispatchTime.m_tick = 0;
    }
    m_entries = m_buffer;
    m_entryNum = static_cast<int>(entryNum);
    m_head = 0;
    m_tail = 0;
    return true;
}

void LatencyEmulator::init(unsigned int entryNum)
{
    m_buffer = 0;
    m_head = 0;
    m_tail = 0;
    allocateBuffer(entryNum);
}

LatencyEmulator::LatencyEmulator(unsigned int entryNum)
    : m_entries(0), m_entryNum(0), m_head(0), m_tail(0), m_buffer(0),
      m_packetInput(0), m_packetOutput(0), m_minimumLatency(0),
      m_maximumLatency(0), m_criticalSection(-1), m_random(), m_entry()
{
    init(entryNum);
}

void LatencyEmulator::freeBuffer()
{
    if (m_buffer == 0)
        return;

    const unsigned int bytes = pead::getArraySize(m_buffer);
    const unsigned int count = bytes / sizeof(Entry);
    for (unsigned int index = 0; index < count; ++index)
        m_buffer[index].~Entry();
    pead::freeToSeadHeap(m_buffer);
    m_buffer = 0;
    m_entries = 0;
}

LatencyEmulator::~LatencyEmulator()
{
    freeBuffer();
}

void LatencyEmulator::SetInput(common::IPacketInput* packetInput)
{
    m_packetInput = packetInput;
}

void LatencyEmulator::SetOutput(common::IPacketOutput* packetOutput)
{
    m_packetOutput = packetOutput;
}

void LatencyEmulator::Clear()
{
    m_tail = 0;
    m_head = 0;
}

Result LatencyEmulator::Read(common::Packet* packet)
{
    if (packet == 0)
        return static_cast<Result>(0x10c07);
    if (m_packetInput != 0)
        return static_cast<Result>(0x10c08);
    if (m_entries == 0)
        return static_cast<Result>(0x2c09);

    common::Time now;
    now.SetNow();
    if (m_tail < 1)
        return static_cast<Result>(0x2c09);

    int index = m_head;
    if (index >= m_entryNum)
        index -= m_entryNum;
    Entry& entry = m_entries[index];
    if (now < entry.m_dispatchTime)
        return static_cast<Result>(0x2c09);

    packet->CopyFrom(entry.m_packet);
    ++m_head;
    if (m_head >= m_entryNum)
        m_head = 0;
    --m_tail;
    return 0;
}

void LatencyEmulator::getLatency(int* minimumLatency, int* maximumLatency) const
{
    m_criticalSection.Lock();
    *minimumLatency = m_minimumLatency;
    *maximumLatency = m_maximumLatency;
    m_criticalSection.Unlock();
}

int LatencyEmulator::calcLatency()
{
    int minimumLatency = 0;
    int maximumLatency = 0;
    getLatency(&minimumLatency, &maximumLatency);
    const unsigned int range =
        static_cast<unsigned int>(maximumLatency - minimumLatency);
    const unsigned long long scaled =
        static_cast<unsigned long long>(m_random.getU32()) * range;
    return static_cast<int>(scaled >> 32) + minimumLatency;
}

Result LatencyEmulator::Write(const common::Packet& packet)
{
    common::Time now;
    now.SetNow();
    const int latency = calcLatency();
    const unsigned long long ticks =
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) *
        static_cast<unsigned int>(latency);
    Entry temporary;
    temporary.m_dispatchTime.m_tick = now.m_tick + ticks;
    temporary.m_packet.CopyFrom(packet);

    if (m_tail >= m_entryNum)
        return static_cast<Result>(0x4c0d);
    int index = m_head + m_tail;
    if (index >= m_entryNum)
        index -= m_entryNum;
    ++m_tail;
    m_entries[index].m_packet.CopyFrom(temporary.m_packet);
    m_entries[index].m_dispatchTime = temporary.m_dispatchTime;
    return 0;
}

Result LatencyEmulator::Dispatch()
{
    if (m_packetInput == 0)
    {
        if (m_packetOutput != 0)
            return writeDispatch();
    }
    else if (m_packetOutput == 0)
    {
        return readDispatch();
    }
    return static_cast<Result>(0x10c08);
}

Result LatencyEmulator::writeDispatch()
{
    common::Time now;
    now.SetNow();
    Result result = 0;

    while (m_tail > 0)
    {
        int index = m_head;
        if (index >= m_entryNum)
            index -= m_entryNum;
        Entry* entry = &m_entries[index];
        if (now < entry->m_dispatchTime)
            return result;

        result = m_packetOutput->Write(entry->m_packet);
        if (m_tail > 0)
        {
            m_head++;
            if (m_head >= m_entryNum)
                m_head = 0;
            m_tail--;
        }
        if (result != 0)
        {
            if (result == static_cast<Result>(0x4c0d))
                result = 0;
            break;
        }
    }
    return result;
}

Result LatencyEmulator::readDispatch()
{
    common::Time now;
    now.SetNow();
    Result result = 0;

    if (m_tail >= m_entryNum)
        return result;

    while (true)
    {
        result = m_packetInput->Read(&m_entry.m_packet);
        if (result != 0)
        {
            if (result != static_cast<Result>(0x2c09))
                return result;
            return 0;
        }

        const unsigned int latency = calcLatency();
        const unsigned long long ticks =
            static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * latency;
        m_entry.m_dispatchTime.m_tick = now.m_tick + ticks;

        if (m_tail >= m_entryNum)
            break;

        int index = m_head + m_tail;
        m_tail++;
        if (index >= m_entryNum)
            index -= m_entryNum;

        Entry& slot = m_entries[index];
        slot.m_packet.CopyFrom(m_entry.m_packet);
        slot.m_dispatchTime = m_entry.m_dispatchTime;

        if (m_tail >= m_entryNum)
            return 0;
    }
    return 0;
}

}}}
