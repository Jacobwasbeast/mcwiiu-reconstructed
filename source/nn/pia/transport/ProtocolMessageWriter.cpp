#include "nn/pia/transport/ProtocolMessageWriter.hpp"

#include <string.h>

namespace nn { namespace pia { namespace transport {

ProtocolMessageWriter::ProtocolMessageWriter()
    : m_source(0), m_protocolId(g_piaProtocolDefaultType)
{
}

ProtocolMessageWriter::~ProtocolMessageWriter()
{
}

void ProtocolMessageWriter::SetSource(unsigned long long source)
{
    m_source = source;
}

void ProtocolMessageWriter::Reset(const ProtocolId& protocolId, unsigned int messageSize,
                                  bool isReliable, bool isPriority)
{
    m_messageSize = messageSize;
    m_protocolId = protocolId;
    m_isReliable = isReliable;
    m_isPriority = isPriority;
    memset(m_field12, 0, sizeof(m_field12));
    m_messageBufferNum = 0;
    m_priority = 0;
}

void ProtocolMessageWriter::AddMessageBuffer(common::Packet* packet, void* buffer,
                                             unsigned long long destinationStationId,
                                             bool field10, bool field11)
{
    MessageBuffer& messageBuffer = m_messageBuffers[m_messageBufferNum];
    messageBuffer.m_packet = packet;
    messageBuffer.m_buffer = buffer;
    messageBuffer.m_destinationStationId = destinationStationId;
    messageBuffer.m_field10 = field10;
    messageBuffer.m_field11 = field11;
    ++m_messageBufferNum;
}

void ProtocolMessageWriter::SetPayload(const void* payload)
{
    memcpy(static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize,
           payload, m_messageSize);
}

void ProtocolMessageWriter::SetPayload(const void* payload, unsigned int offset,
                                       unsigned int size)
{
    memcpy(static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset,
           payload, size);
}

void ProtocolMessageWriter::WriteU8(unsigned char value, unsigned int offset)
{
    unsigned char* buffer =
        static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset;
    buffer[0] = value;
}

void ProtocolMessageWriter::WriteU32(unsigned int value, unsigned int offset)
{
    unsigned char* buffer =
        static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset;
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>(value >> 16);
    buffer[2] = static_cast<unsigned char>(value >> 8);
    buffer[3] = static_cast<unsigned char>(value);
}

void ProtocolMessageWriter::WriteU64(unsigned long long value, unsigned int offset)
{
    unsigned char* buffer =
        static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset;
    const unsigned int high = static_cast<unsigned int>(value >> 32);
    const unsigned int low = static_cast<unsigned int>(value);
    buffer[0] = static_cast<unsigned char>(high >> 24);
    buffer[1] = static_cast<unsigned char>(high >> 16);
    buffer[2] = static_cast<unsigned char>(high >> 8);
    buffer[3] = static_cast<unsigned char>(high);
    buffer[4] = static_cast<unsigned char>(low >> 24);
    buffer[5] = static_cast<unsigned char>(low >> 16);
    buffer[6] = static_cast<unsigned char>(low >> 8);
    buffer[7] = static_cast<unsigned char>(low);
}

}}}
