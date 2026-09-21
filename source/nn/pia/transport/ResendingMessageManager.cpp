#include "nn/pia/transport/ResendingMessageManager.hpp"

#include <cstring>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

ResendingMessageManager* g_piaResendingMessageManager = 0;

namespace
{
void* g_resendingMessageManagerStorage = 0;
const unsigned long long cResendInterval =
    static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick) * 500;
}

ResendingMessageManager::ResendingMessageManager()
    : m_nextSendTimes(0), m_timeouts(0), m_ackIds(0), m_messages(0), m_messageSizes(0),
      m_field14(0), m_field18(0), m_field1c(0), m_field20(0), m_field2c(0),
      m_entryNum(0)
{
}

ResendingMessageManager::~ResendingMessageManager()
{
}

Result ResendingMessageManager::CreateInstance()
{
    if (g_piaResendingMessageManager != 0)
        return static_cast<Result>(0x10c0f);

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_resendingMessageManagerStorage =
        pead::allocateFromSeadHeap(sizeof(ResendingMessageManager), heap, 8);
    if (g_resendingMessageManagerStorage == 0)
        return static_cast<Result>(0x10c01);

    g_piaResendingMessageManager =
        new (g_resendingMessageManagerStorage) ResendingMessageManager();
    return 0;
}

Result ResendingMessageManager::Startup(PacketHandler* packetHandler, bool isRelay)
{
    if (packetHandler == 0)
        return static_cast<Result>(0x10c07);

    m_packetHandler = packetHandler;
    m_isRelay = isRelay;
    return 0;
}

Result ResendingMessageManager::SetSendMessageCommon(unsigned int* ackId,
                                                     const unsigned char* message,
                                                     unsigned int messageSize,
                                                     ProtocolId protocolId,
                                                     bool isRelay, long long timeout,
                                                     unsigned int* index)
{
    const unsigned int payloadSizeLimit =
        static_cast<unsigned int>(m_packetHandler->GetPayloadSizeLimit());
    if (messageSize > payloadSizeLimit - 4U)
        return static_cast<Result>(0x10c07);

    common::Time now;
    now.SetNow();
    if (timeout != 0 && timeout <= static_cast<long long>(now.m_tick))
        return static_cast<Result>(0x10c07);

    unsigned int selected = 0;
    if (m_entryNum == 0)
        return static_cast<Result>(0x4c0d);
    while (selected < m_entryNum && m_ackIds[selected] != 0)
        ++selected;
    if (selected == m_entryNum)
        return static_cast<Result>(0x4c0d);

    if (m_field2c == 0)
    {
        const unsigned int nowLow = static_cast<unsigned int>(now.m_tick);
        m_field2c = nowLow != 0 ? nowLow : 1;
    }

    std::memcpy(m_messages[selected], message, messageSize);
    common::serializeU32(m_messages[selected] + messageSize, m_field2c);
    m_messageSizes[selected] = messageSize + 4;
    m_field1c[selected] = protocolId;
    m_field20[selected] = static_cast<unsigned char>(isRelay);
    m_nextSendTimes[selected] = now.m_tick;
    m_timeouts[selected] = static_cast<unsigned long long>(timeout);
    m_ackIds[selected] = m_field2c;

    ++m_field2c;
    if (m_field2c == 0)
        m_field2c = 1;

    *ackId = m_ackIds[selected];
    *index = selected;
    return 0;
}

Result ResendingMessageManager::SetSendMessageByStationAddress(
    unsigned int* ackId, const unsigned char* message, unsigned int messageSize,
    const common::StationAddress& stationAddress, ProtocolId protocolId, long long timeout)
{
    unsigned int index;
    const Result result = SetSendMessageCommon(ackId, message, messageSize, protocolId, true,
                                               timeout, &index);
    if (result == 0)
    {
        m_field18[index] = stationAddress;
        m_field14[index] = 253;
    }
    return result;
}

Result ResendingMessageManager::SetSendMessageByStationIndex(
    unsigned int* ackId, const unsigned char* message, unsigned int messageSize,
    StationIndex stationIndex, ProtocolId protocolId, bool isRelay, long long timeout)
{
    if (m_isRelay)
        return static_cast<Result>(0x10c08);

    unsigned int index;
    const Result result = SetSendMessageCommon(ackId, message, messageSize, protocolId, isRelay,
                                               timeout, &index);
    if (result == 0)
    {
        m_field18[index].Clear();
        m_field14[index] = static_cast<unsigned int>(stationIndex);
    }
    return result;
}

Result ResendingMessageManager::Dispatch()
{
    common::Time now;
    now.SetNow();
    if (m_packetHandler == 0)
        return 0;

    const long long currentTime = static_cast<long long>(now.m_tick);
    for (unsigned int index = 0; index < m_entryNum; ++index)
    {
        if (m_ackIds[index] == 0)
            continue;
        if (m_timeouts[index] != 0 &&
            currentTime >= static_cast<long long>(m_timeouts[index]))
        {
            m_ackIds[index] = 0;
            m_timeouts[index] = 0;
            m_nextSendTimes[index] = 0;
            continue;
        }
        if (currentTime < static_cast<long long>(m_nextSendTimes[index]))
            continue;

        ProtocolMessageWriter* writer;
        if (static_cast<int>(m_field14[index]) < 32)
        {
            writer = static_cast<StationPacketHandler*>(m_packetHandler)->AssignByStationIndex(
                m_field1c[index], static_cast<StationIndex>(m_field14[index]),
                m_messageSizes[index], false, false, m_field20[index] != 0);
        }
        else
        {
            writer = m_packetHandler->AssignByStationAddress(m_field1c[index],
                m_field18[index], m_messageSizes[index], false, false);
        }
        if (writer != 0)
        {
            writer->SetPayload(m_messages[index]);
            m_packetHandler->Commit();
            m_nextSendTimes[index] = now.m_tick + cResendInterval;
        }
    }
    return 0;
}

bool ResendingMessageManager::CheckNowResending(unsigned int ackId) const
{
    if (ackId == 0)
        return false;

    for (unsigned int i = 0; i < m_entryNum; ++i)
    {
        if (m_ackIds[i] == ackId)
            return true;
    }
    return false;
}

}}}
