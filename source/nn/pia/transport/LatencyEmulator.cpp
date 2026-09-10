#include "nn/pia/transport/LatencyEmulator.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace transport {

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
