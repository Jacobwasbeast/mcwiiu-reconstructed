#include "nn/pia/transport/ReceiveThreadStream.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/transport/LatencyEmulator.hpp"

namespace nn { namespace pia { namespace transport {

namespace {
common::Packet* assignPacket(ReceiveThreadStream* stream)
{
    const unsigned int count = stream->m_packetNum;
    if (count == 0)
        return 0;
    const unsigned int next = static_cast<unsigned int>(stream->m_writer.m_field08 + 1) % count;
    if (count < 2)
        return 0;
    const unsigned int index = static_cast<unsigned int>(stream->m_writer.m_field08 < 0 ? 0 : stream->m_writer.m_field08);
    stream->m_writer.m_field08 = static_cast<int>(next);
    ++stream->m_writer.m_field0c;
    return &stream->m_packets[index];
}

common::Packet* getPacket(ReceiveThreadStream* stream)
{
    const unsigned int count = stream->m_packetNum;
    if (count == 0 || stream->m_writer.m_field04 < 0)
        return 0;
    unsigned int index = static_cast<unsigned int>(stream->m_writer.m_field04);
    if (index >= count)
        index %= count;
    return &stream->m_packets[index];
}

void pushPacket(ReceiveThreadStream* stream)
{
    stream->m_writer.m_field04 = stream->m_writer.m_field08;
    stream->m_writer.m_field0c = 0;
}
}

ReceiveThreadStream::ReceiveThreadStream()
    : TransportThreadStream(), m_packetInput(0), m_receivePacketNum(0), m_receiveDataSize(0)
{
}

ReceiveThreadStream::~ReceiveThreadStream()
{
}

Result ReceiveThreadStream::Initialize(common::IPacketInput* packetInput,
                                       unsigned int packetNum, int priority,
                                       unsigned int latencyEntryNum,
                                       bool isPacketLossEnabled)
{
    if (m_packetInput != 0)
        return 0x10c02;
    if (packetInput == 0)
        return 0x10c07;
    const Result result = InitializeCore("Pia_ReceiveThreadStream", priority,
                                         packetNum, latencyEntryNum,
                                         isPacketLossEnabled, 1);
    if (result != 0)
        return result;
    if (m_latencyEmulator != 0)
        m_latencyEmulator->SetInput(packetInput);
    m_packetInput = packetInput;
    m_receivePacketNum = 0;
    m_receiveDataSize = 0;
    m_writer.m_field04 = 0;
    m_writer.m_field08 = 0;
    m_writer.m_field0c = 0;
    return 0;
}

void ReceiveThreadStream::Finalize()
{
    FinalizeCore();
    m_receivePacketNum = 0;
    m_packetInput = 0;
}

void ReceiveThreadStream::SetMonitoringData()
{
    common::g_sessionStateMonitoringContent.m_receiveThreadSleepMilliSecond = 0xff;
    if (static_cast<unsigned int>(m_sleepMilliSecond) - 1u < 0xfe)
        common::g_sessionStateMonitoringContent.m_receiveThreadSleepMilliSecond = static_cast<unsigned char>(m_sleepMilliSecond);
}

void ReceiveThreadStream::OnStartup()
{
    common::g_sessionStateMonitoringContent.m_receiveThreadMetric = 0;
}

Result ReceiveThreadStream::ProcessOne()
{
    common::Packet* packet = getPacket(this);
    if (packet == 0)
        packet = assignPacket(this);
    if (packet == 0)
        return 0x4c0d;

    Result result = 0;
    if (m_latencyEmulator == 0)
        result = m_packetInput->Read(packet);
    else
        result = m_latencyEmulator->Read(packet);
    if (result != 0)
        return result;

    packet->m_packetData.DeserializeHeader();
    packet->m_field603 = packet->m_packetData.m_header.m_version != 1;
    if (isDropPacket())
        return 0x2c09;

    ++m_receivePacketNum;
    m_receiveDataSize += packet->m_size;
    if (packet->IsValid() == 0)
        return 0x2c03;
    pushPacket(this);
    return 0;
}

}}}
