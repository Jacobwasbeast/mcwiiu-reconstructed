#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/ReceivedDataContainer.hpp"
#include "nn/pia/ReceivedData.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>
#include <string.h>

namespace nn { namespace pia { namespace transport {

unsigned char g_piaTransportInitialized = 0;
unsigned char g_piaTransportInSetupMode = 0;
unsigned short g_piaProtocolDefaultType = 0;
ProtocolManager* g_piaProtocolManager = 0;
unsigned int g_receivedDataCount = 0;

unsigned char IsInitialized()
{
    return g_piaTransportInitialized;
}

unsigned char IsInSetupMode()
{
    return g_piaTransportInSetupMode;
}


Protocol::Protocol()
    : mProtocolId(g_piaProtocolDefaultType)
{
    prev = 0;
    next = 0;
}

Protocol::~Protocol()
{
}

void Protocol::Trace(unsigned long long) const
{
}

unsigned short Protocol::GetProtocolType() const
{
    return 0;
}

Result Protocol::StartupWithStationAddress(const common::StationAddress&)
{
    return 0;
}

Result Protocol::Startup()
{
    return 0;
}

void Protocol::SetPort(unsigned char port)
{
    const unsigned short type = GetProtocolType();
    mProtocolId = ProtocolId(static_cast<unsigned short>((port & 0xff) | (type << 8)));
}

unsigned int Protocol::GetStationPacketHandler()
{
    return mStationPacketHandler;
}

void Protocol::CleanupWithStationAddress()
{
}

void Protocol::Cleanup()
{
}

Result Protocol::Dispatch()
{
    return 0;
}

Result Protocol::UpdateProtocolEvent(const ProtocolEvent&)
{
    return 0;
}

unsigned int Protocol::IsProtocolFilteringEnabled() const
{
    return 1;
}

UnreliableProtocol::UnreliableProtocol()
    : Protocol(), m_receivedData(0), m_receivedDataContainer(0)
{
}

UnreliableProtocol::~UnreliableProtocol()
{
}

void UnreliableProtocol::Trace(unsigned long long) const
{
}

unsigned short UnreliableProtocol::GetProtocolType() const
{
    return cProtocolType;
}

unsigned int UnreliableProtocol::Initialize(unsigned int count)
{
    if (count == 0) return 0x10c07;
    if (IsInitialized() == 0) return 0x10c0c;
    if (m_receivedDataContainer != 0) return 0x10c02;
    pead::Heap* heap = common::HeapManager::GetHeap();
    m_receivedDataContainer = static_cast<ReceivedDataContainer*>(
        pead::allocateFromSeadHeap(sizeof(ReceivedDataContainer), heap));
    if (m_receivedDataContainer == 0) return 0x10c01;
    new (m_receivedDataContainer) ReceivedDataContainer();
    m_receivedData = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(count * 0x598, heap));
    if (m_receivedData == 0)
    {
        m_receivedDataContainer->~ReceivedDataContainer();
        pead::freeToSeadHeap(m_receivedDataContainer);
        m_receivedDataContainer = 0;
        return 0x10c01;
    }
    static_cast<ReceivedDataContainer*>(m_receivedDataContainer)->m_storage = m_receivedData;
    static_cast<ReceivedDataContainer*>(m_receivedDataContainer)->m_nodeOffset = 0;
    static_cast<ReceivedDataContainer*>(m_receivedDataContainer)->m_count = count;
    memset(m_receivedData, 0, count * 0x598);
    ReceivedDataContainer* container =
        static_cast<ReceivedDataContainer*>(m_receivedDataContainer);
    const unsigned int listCount = count < 2 ? 1 : count;
    for (unsigned int i = 0; i < listCount; ++i)
    {
        ReceivedData* element = reinterpret_cast<ReceivedData*>(m_receivedData + i * 0x598);
        element->node.prev = 0;
        element->node.next = 0;
        common::ListNode* sentinel = container->m_available.Sentinel();
        container->m_available.InsertBeforeNode(sentinel, &element->node);
    }
    g_receivedDataCount = count;
    return 0;
}

void UnreliableProtocol::Finalize()
{
    Cleanup();
}

}}}
