#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <coreinit/systeminfo.h>
#include <coreinit/time.h>
#include <new>
#include <string.h>

namespace nn { namespace pia { namespace transport {

unsigned short g_reliableSlidingWindowGeneration = 0;

ReliableSlidingWindow::MessageHeader::~MessageHeader()
{
}

Result ReliableSlidingWindow::MessageHeader::Serialize(unsigned char* buffer,
                                                       unsigned int* size) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    buffer[0] = static_cast<unsigned char>(m_flags >> 8);
    buffer[1] = static_cast<unsigned char>(m_flags);
    buffer[2] = static_cast<unsigned char>(m_size >> 8);
    buffer[3] = static_cast<unsigned char>(m_size);
    const unsigned int words[] = { m_field08, m_sequenceId, m_field10 };
    for (unsigned int index = 0; index < 3; ++index)
    {
        buffer[4 + index * 4] = static_cast<unsigned char>(words[index] >> 24);
        buffer[5 + index * 4] = static_cast<unsigned char>(words[index] >> 16);
        buffer[6 + index * 4] = static_cast<unsigned char>(words[index] >> 8);
        buffer[7 + index * 4] = static_cast<unsigned char>(words[index]);
    }
    for (unsigned int index = 0; index < 8; ++index)
        buffer[16 + index] = static_cast<unsigned char>(m_field18 >> (56 - index * 8));
    *size = cHeaderSize;
    return 0;
}

Result ReliableSlidingWindow::MessageHeader::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    m_flags = static_cast<unsigned short>((buffer[0] << 8) | buffer[1]);
    m_size = static_cast<unsigned short>((buffer[2] << 8) | buffer[3]);
    unsigned int* words[] = { &m_field08, &m_sequenceId, &m_field10 };
    for (unsigned int index = 0; index < 3; ++index)
        *words[index] = (static_cast<unsigned int>(buffer[4 + index * 4]) << 24) |
            (static_cast<unsigned int>(buffer[5 + index * 4]) << 16) |
            (static_cast<unsigned int>(buffer[6 + index * 4]) << 8) | buffer[7 + index * 4];
    m_field18 = 0;
    for (unsigned int index = 0; index < 8; ++index)
        m_field18 = (m_field18 << 8) | buffer[16 + index];
    return 0;
}

unsigned int ReliableSlidingWindow::MessageHeader::GetSerializedSize() const
{
    return cHeaderSize;
}

void ReliableSlidingWindow::MessageData::DeserializeHeader(const void* buffer)
{
    m_header.Deserialize(static_cast<const unsigned char*>(buffer));
}

void ReliableSlidingWindow::MessageData::SerializeHeader()
{
    unsigned int size;
    m_header.Serialize(m_data, &size);
}

void ReliableSlidingWindow::MessageData::CopyPayload(void* destination, const void* source,
                                                     unsigned int size)
{
    memmove(destination, source, size);
}

void ReliableSlidingWindow::MessageData::SetPayload(const void* data, unsigned int size)
{
    memmove(m_data + cHeaderSize, data, size);
}

template <typename T>
ReliableSlidingWindow::Buffer<T>::Buffer()
    : m_data(0), m_capacity(0), m_head(0), m_sequence(0), m_count(0)
{
}

template <typename T>
ReliableSlidingWindow::Buffer<T>::~Buffer()
{
    Finalize();
}

template <typename T>
Result ReliableSlidingWindow::Buffer<T>::Initialize(unsigned int capacity)
{
    if (m_data != 0)
        return 0x10c02;
    if (capacity == 0)
        return 0x10c07;
    T* data = static_cast<T*>(pead::allocateFromSeadHeap(capacity * sizeof(T), common::HeapManager::GetHeap()));
    if (data != 0)
    {
        for (unsigned int index = 0; index < capacity; ++index)
            new (data + index) T();
    }
    m_data = data;
    m_capacity = capacity;
    return 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Finalize()
{
    if (m_data != 0)
    {
        pead::getArraySize(m_data);
        pead::freeToSeadHeap(m_data);
        m_data = 0;
    }
    m_capacity = 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Clear()
{
    m_head = 0;
    m_sequence = 0xfffff82fu;
    m_count = 0;
}

template <typename T>
void ReliableSlidingWindow::Buffer<T>::Withdraw(unsigned int count)
{
    m_head += count;
    if (m_head >= m_capacity)
        m_head -= m_capacity;
    m_sequence += count;
    m_count -= count;
}

template <typename T>
T* ReliableSlidingWindow::Buffer<T>::GetData(unsigned int sequence)
{
    const unsigned int offset = sequence - m_sequence;
    if (static_cast<int>(offset) < 0 || offset >= m_count)
        return 0;
    unsigned int index = m_head + offset;
    if (index >= m_capacity)
        index -= m_capacity;
    return m_data + index;
}

template class ReliableSlidingWindow::Buffer<ReliableSlidingWindow::SendData>;
template class ReliableSlidingWindow::Buffer<ReliableSlidingWindow::ReceiveData>;

ReliableSlidingWindow::SendData* ReliableSlidingWindow::SendBuffer::Assign()
{
    unsigned int index = m_head + m_count;
    if (index >= m_capacity)
        index -= m_capacity;
    SendData* data = m_data + index;
    data->m_message.m_header.m_flags = 1;
    data->m_message.m_header.m_sequenceId = m_sequence + m_count;
    ++m_count;
    return data;
}

ReliableSlidingWindow::ReceiveData* ReliableSlidingWindow::ReceiveBuffer::Assign(unsigned int sequence)
{
    const unsigned int offset = sequence - m_sequence;
    while (m_count <= offset)
    {
        unsigned int index = m_head + m_count;
        if (index >= m_capacity)
            index -= m_capacity;
        m_data[index].m_valid = false;
        ++m_count;
    }
    unsigned int index = m_head + offset;
    if (index >= m_capacity)
        index -= m_capacity;
    return m_data + index;
}

ReliableSlidingWindow::ReliableSlidingWindow()
    : m_generation(g_reliableSlidingWindowGeneration), m_localStationId(0), m_remoteStationId(0),
      m_payloadLimit(0), m_byteCount(0), m_profileStart(0)
{
}

ReliableSlidingWindow::~ReliableSlidingWindow()
{
    Finalize();
}

void ReliableSlidingWindow::Finalize()
{
    m_receiveBuffer.Finalize();
    m_sendBuffer.Finalize();
}

Result ReliableSlidingWindow::Initialize(unsigned int sendCapacity, unsigned int receiveCapacity)
{
    Result result = m_sendBuffer.Initialize(sendCapacity);
    if (result != 0)
        return result;
    result = m_receiveBuffer.Initialize(receiveCapacity);
    if (result != 0)
    {
        m_sendBuffer.Finalize();
        return result;
    }
    const unsigned long long ticksPerSecond = OSGetSystemInfo()->busClockSpeed >> 2;
    m_retrySpan = ticksPerSecond * 500 / 1000;
    return 0;
}

Result ReliableSlidingWindow::Startup(StationPacketHandler* handler, unsigned short generation,
                                      unsigned long long localStationId,
                                      unsigned long long remoteStationId)
{
    if (m_sendBuffer.m_capacity == 0)
        return 0x10c0c;
    if (IsInCommunication())
        return 0x10c08;
    if (generation == g_reliableSlidingWindowGeneration || localStationId == 0 ||
        remoteStationId == 0 || localStationId == remoteStationId)
        return 0x10c07;
    m_localStationId = localStationId;
    m_generation = generation;
    m_remoteStationId = remoteStationId;
    m_payloadLimit = (handler->m_payloadSize & ~7u) - cHeaderSize;
    m_sendBuffer.Clear();
    m_receiveBuffer.Clear();
    m_time = static_cast<unsigned long long>(OSGetSystemTime());
    m_sequence = 0xfffff82fu;
    m_receivedBits = 0;
    m_field68 = false;
    m_byteCount = 0;
    m_profileStart = 0;
    return 0;
}

void ReliableSlidingWindow::Cleanup()
{
    if (m_generation == g_reliableSlidingWindowGeneration)
        return;
    m_generation = g_reliableSlidingWindowGeneration;
    m_payloadLimit = 0;
    m_remoteStationId = 0;
    m_localStationId = 0;
}

bool ReliableSlidingWindow::IsInCommunication() const
{
    return m_generation != g_reliableSlidingWindowGeneration;
}

unsigned int ReliableSlidingWindow::CanPushData(unsigned int size)
{
    if (!IsInCommunication())
        return false;
    const unsigned int count = size == 0 ? 1 : (size - 1) / m_payloadLimit + 1;
    return count <= m_sendBuffer.m_capacity - m_sendBuffer.m_count;
}

Result ReliableSlidingWindow::PushData(const void* data, unsigned int size)
{
    if (!IsInCommunication())
        return 0x10c08;
    if (size != 0 && data == 0)
        return 0x10c07;
    const unsigned int count = size == 0 ? 1 : (size - 1) / m_payloadLimit + 1;
    if (count > m_sendBuffer.m_capacity - m_sendBuffer.m_count)
        return 0x4c0d;
    if (m_byteCount != 0 || size >= 0x19000)
    {
        if (m_byteCount == 0)
            m_profileStart = g_piaTransport->m_time.m_tick;
        m_byteCount += size;
    }
    const unsigned char* source = static_cast<const unsigned char*>(data);
    for (unsigned int index = 0; index < count; ++index)
    {
        SendData* fragment = m_sendBuffer.Assign();
        const unsigned int fragmentSize = index + 1 == count ? size : m_payloadLimit;
        if (fragmentSize != 0)
            fragment->m_message.SetPayload(source, fragmentSize);
        fragment->m_message.m_header.m_size = static_cast<unsigned short>(fragmentSize);
        fragment->m_sendTime = 0;
        fragment->m_size = fragmentSize + cHeaderSize;
        fragment->m_field0c = 0;
        if (index + 1 == count)
            fragment->m_message.m_header.m_flags |= 2;
        else
        {
            source += fragmentSize;
            size -= fragmentSize;
        }
    }
    return 0;
}

Result ReliableSlidingWindow::PopData(void* buffer, unsigned int* size, unsigned int bufferSize)
{
    if (!IsInCommunication())
        return 0x10c08;
    if (buffer == 0 || size == 0)
        return 0x10c07;
    unsigned int total = 0;
    unsigned int count = 0;
    while (count < m_receiveBuffer.m_count)
    {
        unsigned int index = m_receiveBuffer.m_head + count;
        if (index >= m_receiveBuffer.m_capacity)
            index -= m_receiveBuffer.m_capacity;
        ReceiveData* fragment = m_receiveBuffer.m_data + index;
        if (!fragment->m_valid)
            return 0x2c09;
        total += fragment->m_size;
        if (fragment->m_end)
        {
            if (bufferSize < total)
                return 0x2c03;
            unsigned char* destination = static_cast<unsigned char*>(buffer);
            for (unsigned int offset = 0; offset <= count; ++offset)
            {
                index = m_receiveBuffer.m_head + offset;
                if (index >= m_receiveBuffer.m_capacity)
                    index -= m_receiveBuffer.m_capacity;
                fragment = m_receiveBuffer.m_data + index;
                MessageData::CopyPayload(destination, fragment->m_payload, fragment->m_size);
                destination += fragment->m_size;
            }
            *size = total;
            m_receiveBuffer.Withdraw(count + 1);
            return 0;
        }
        ++count;
    }
    return count < m_receiveBuffer.m_capacity ? 0x2c09 : 0x2c03;
}

void ReliableSlidingWindow::Trace(unsigned long long) const
{
}

}}}
