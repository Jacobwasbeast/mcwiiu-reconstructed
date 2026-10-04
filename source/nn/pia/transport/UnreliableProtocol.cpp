#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "nn/pia/ReceivedDataContainer.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>
#include <stddef.h>
#include <string.h>

namespace nn { namespace pia { namespace common {

bool isValidDestinationStationIndex(StationIndex stationIndex);

}}}

namespace nn { namespace pia { namespace transport {

unsigned int g_receivedDataCount = 0;

namespace
{

inline UnreliableProtocol::ReceivedMessage* messageOf(UnreliableProtocol::Payload* payload)
{
    return reinterpret_cast<UnreliableProtocol::ReceivedMessage*>(
        reinterpret_cast<char*>(payload) - offsetof(UnreliableProtocol::ReceivedMessage, payload));
}

inline common::ListNode* nodeOf(const ReceivedDataContainer* container,
                                UnreliableProtocol::ReceivedMessage* message)
{
    return reinterpret_cast<common::ListNode*>(
        reinterpret_cast<char*>(message) + container->m_nodeOffset);
}

inline UnreliableProtocol::Payload* payloadOf(common::ListNode* node)
{
    return &reinterpret_cast<UnreliableProtocol::ReceivedMessage*>(node)->payload;
}

inline UnreliableProtocol::Payload* firstPayload(ReceivedDataContainer* container)
{
    if (container->m_list.GetNodeNum() == 0)
        return 0;
    common::ListNode* node = container->m_list.Sentinel()->next;
    if (node == 0)
        return 0;
    return payloadOf(node);
}

inline UnreliableProtocol::Payload* nextPayload(ReceivedDataContainer* container,
                                                UnreliableProtocol::Payload* payload)
{
    common::ListNode* next = messageOf(payload)->node.next;
    if (next == container->m_list.Sentinel())
        return 0;
    return payloadOf(next);
}

inline void recyclePayload(ReceivedDataContainer* container,
                           UnreliableProtocol::Payload* payload)
{
    UnreliableProtocol::ReceivedMessage* message = messageOf(payload);
    container->m_list.EraseNode(&message->node);
    container->m_available.InsertAfterNode(container->m_available.Sentinel(),
                                           nodeOf(container, message));
}

inline void updateWatermark(const ReceivedDataContainer* container)
{
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(9)->Update(container->m_list.GetNodeNum());
}

}

UnreliableProtocol::UnreliableProtocol()
    : Protocol(),
      m_receivedData(0),
      m_receivedDataContainer(0),
      m_stationIndex(cInvalidStationIndex)
{
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(9)->SetName("UnreliableProtocol receive buffer num");
}

UnreliableProtocol::~UnreliableProtocol()
{
    if (m_receivedDataContainer != 0 || m_receivedData != 0)
        Finalize();
}

void UnreliableProtocol::Trace(unsigned long long) const
{
}

unsigned short UnreliableProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result UnreliableProtocol::Initialize(unsigned int count)
{
    if (count == 0)
        return 0x10c07;
    if (!IsInitialized())
        return 0x10c0c;
    if (m_receivedDataContainer != 0)
        return 0x10c02;

    pead::Heap* heap = common::HeapManager::GetHeap();
    void* containerMemory = pead::allocateFromSeadHeap(sizeof(ReceivedDataContainer), heap);
    if (containerMemory == 0)
    {
        m_receivedDataContainer = 0;
        return 0x10c01;
    }
    m_receivedDataContainer = new (containerMemory) ReceivedDataContainer();

    unsigned int storageSize = count * sizeof(ReceivedMessage);
    unsigned char* storage = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(storageSize, common::HeapManager::GetHeap()));
    if (storage != 0)
    {
        for (unsigned int index = 0; index < storageSize; ++index)
            new (storage + index) unsigned char();
    }
    m_receivedData = storage;
    if (storage == 0)
    {
        common::DeleteObj(m_receivedDataContainer);
        m_receivedDataContainer = 0;
        return 0x10c01;
    }

    ReceivedDataContainer* container = m_receivedDataContainer;
    container->m_storage = storage;
    container->m_count = count;
    unsigned int remaining = count < 2 ? 1 : count;
    unsigned int offset = 0;
    do
    {
        ReceivedMessage* message = reinterpret_cast<ReceivedMessage*>(container->m_storage + offset);
        message->node.prev = 0;
        message->node.next = 0;
        container->m_available.InsertBeforeNode(container->m_available.Sentinel(),
                                                nodeOf(container, message));
        --remaining;
        offset += sizeof(ReceivedMessage);
    } while (remaining != 0);
    g_receivedDataCount = count;
    return 0;
}

void UnreliableProtocol::Finalize()
{
    CleanupWithStationAddress();
    if (m_receivedDataContainer != 0)
    {
        common::DeleteObj(m_receivedDataContainer);
        m_receivedDataContainer = 0;
    }
    unsigned char* storage = m_receivedData;
    if (storage != 0)
    {
        pead::getArraySize(storage);
        pead::freeToSeadHeap(storage);
        m_receivedData = 0;
    }
}

Result UnreliableProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    Station* station = g_piaStationManager->GetStation(address);
    if (station == 0)
        return 0x10c07;
    if (station->m_field10 >= StationManager::cStationNumMax)
        return 0x10c07;
    if (m_stationIndex != cInvalidStationIndex)
        return 0x10c08;
    m_stationIndex = station->m_field10;
    return 0;
}

void UnreliableProtocol::CleanupWithStationAddress()
{
    ReceivedDataContainer* container = m_receivedDataContainer;
    m_stationIndex = cInvalidStationIndex;
    if (container != 0)
    {
        common::ListNode* sentinel = container->m_list.Sentinel();
        common::ListNode* node = sentinel->next;
        while (node != sentinel)
        {
            common::ListNode* next = node->next;
            node->prev = 0;
            node->next = 0;
            container->m_available.InsertAfterNode(
                container->m_available.Sentinel(),
                reinterpret_cast<common::ListNode*>(reinterpret_cast<char*>(node) + container->m_nodeOffset));
            node = next;
        }
        container->m_list.Init();
    }
}

Result UnreliableProtocol::Dispatch()
{
    if (mStationPacketHandler == 0)
        return 0x10c08;
    if (m_receivedDataContainer == 0)
        return 0x10c08;

    PacketHandler* handler = reinterpret_cast<PacketHandler*>(mStationPacketHandler);
    ProtocolId protocolId = mProtocolId;
    PacketHandler::Iterator& iterator = handler->GetIterator(protocolId);
    while (!handler->IsEndIteration())
    {
        ReceivedDataContainer* container = m_receivedDataContainer;
        if (container->m_available.GetNodeNum() != 0)
        {
            ProtocolMessageReader* reader = iterator.GetMessageReader();
            Payload* payload = 0;
            if (container->m_available.GetNodeNum() != 0)
            {
                common::ListNode* node = container->m_available.PopBackNode();
                ReceivedMessage* message = 0;
                if (node != 0)
                    message = reinterpret_cast<ReceivedMessage*>(
                        reinterpret_cast<char*>(node) - container->m_nodeOffset);
                new (&message->payload) Payload();
                container->m_list.InsertBeforeNode(container->m_list.Sentinel(), &message->node);
                payload = &message->payload;
            }
            payload->stationId = reader->m_header.m_source;
            payload->size = reader->m_messageSize;
            memcpy(payload->data, reader->m_message + ProtocolMessageReader::cMessageHeaderSize,
                   reader->m_messageSize);
        }
        handler->NextIteration();
    }
    updateWatermark(m_receivedDataContainer);
    return 0;
}

Result UnreliableProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    Station* station = g_piaStationManager->GetStation(event.m_stationAddress);
    unsigned long long stationId =
        (static_cast<unsigned long long>(station->m_field18) << 32) | station->m_field1c;
    ReceivedDataContainer* container = m_receivedDataContainer;
    if (event.m_type > 1 || container->m_list.GetNodeNum() == 0 || container->m_list.Sentinel()->next == 0)
        return 0;
    Payload* payload = firstPayload(container);
    do
    {
        Payload* next = nextPayload(container, payload);
        if (payload->stationId == stationId)
            recyclePayload(container, payload);
        payload = next;
        container = m_receivedDataContainer;
    } while (payload != 0);
    return 0;
}

Result UnreliableProtocol::Receive(unsigned long long* stationId, void* buffer,
                                   unsigned int* size, unsigned int bufferSize)
{
    if (stationId == 0)
        return 0x10c07;
    if (buffer == 0)
        return 0x10c07;
    if (bufferSize == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;

    ReceivedDataContainer* container = m_receivedDataContainer;
    if (container == 0)
        return 0x10c08;
    if (!IsInCommunication())
        return 0x2c26;

    if (container->m_list.GetNodeNum() == 0)
        return 0x2c09;
    Payload* payload = firstPayload(container);
    if (bufferSize < payload->size)
        return 0x10c04;
    payload = firstPayload(container);
    if (payload == 0)
        return 0x2c09;

    while (!g_piaTransport->m_stationIdTable->IsJoinEventNotifiedByStationId(payload->stationId))
    {
        Payload* next = nextPayload(m_receivedDataContainer, payload);
        recyclePayload(m_receivedDataContainer, payload);
        if (next == 0)
            return 0x2c09;
        payload = next;
    }

    if (payload->size < bufferSize)
        bufferSize = payload->size;
    memcpy(buffer, payload->data, bufferSize);
    *size = bufferSize;
    *stationId = payload->stationId;
    recyclePayload(m_receivedDataContainer, payload);
    updateWatermark(m_receivedDataContainer);
    return 0;
}

bool UnreliableProtocol::IsInCommunication() const
{
    return m_stationIndex != cInvalidStationIndex;
}

Result UnreliableProtocol::sendImpl(StationIndex stationIndex, const void* data, unsigned int size)
{
    if (!common::isValidDestinationStationIndex(stationIndex))
        return 0x10c07;
    if (data == 0)
    {
        if (size != 0)
            return 0x10c07;
    }
    else if (size > cReceivedDataSizeMax)
    {
        return 0x10c07;
    }
    if (m_receivedDataContainer == 0)
        return 0x10c08;
    if (stationIndex == static_cast<StationIndex>(cInvalidStationIndex))
        return 0x10c07;

    StationPacketHandler* handler = reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (handler == 0)
        return 0x10c08;
    if (static_cast<unsigned int>(handler->GetPayloadSizeLimit()) < size)
        return 0x10c07;
    if (!IsInCommunication())
        return 0x2c26;
    if (g_piaTransport->m_transportStatus == static_cast<Result>(0x4c11))
        return 0x4c11;

    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer =
        handler->AssignByStationIndex(protocolId, stationIndex, size, false, false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(data);
    return handler->Commit();
}

Result UnreliableProtocol::Send(StationIndex stationIndex, const void* data, unsigned int size)
{
    return sendImpl(stationIndex, data, size);
}

Result UnreliableProtocol::Send(unsigned long long stationId, const void* data, unsigned int size)
{
    StationIndex index = static_cast<StationIndex>(0);
    Result result = g_piaTransport->ConvertToStationIndex(&index, stationId);
    if (result == 0)
        return Send(index, data, size);
    return result;
}

void UnreliableProtocol::SendToAll(const void* data, unsigned int size)
{
    Send(static_cast<StationIndex>(cStationIndexAll), data, size);
}

Result UnreliableProtocol::SendToMulti(unsigned int stationBitmap, const void* data,
                                       unsigned int size)
{
    if (stationBitmap == 0 || (data == 0 && size != 0))
        return 0x10c07;
    if (m_receivedDataContainer == 0)
        return 0x10c08;

    StationPacketHandler* handler = reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (handler == 0)
        return 0x10c08;
    if (static_cast<unsigned int>(handler->GetPayloadSizeLimit()) < size)
        return 0x10c07;
    if (!IsInCommunication())
        return 0x2c26;

    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer =
        handler->AssignByStationBitmap(protocolId, stationBitmap, size, false, false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(data);
    return handler->Commit();
}

}}}
