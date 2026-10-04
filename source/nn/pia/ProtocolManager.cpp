#include "nn/pia/Reconstruction.hpp"
#include "vendor/__N_29_transport_ProtocolManager_cpp_70a76ebe.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>

namespace nn { namespace pia { namespace transport {

unsigned int g_piaProtocolTypeMask = 0;



namespace
{
void DestroyProtocolStorage(Protocol* protocol)
{
    protocol->~Protocol();
    pead::DeleteArray(reinterpret_cast<unsigned char*>(protocol));
}
}

ProtocolManager::ProtocolManager()
    : m_networkFactory(0), m_protocols(), m_nodeOffset(0), m_started(0)
{
}

ProtocolManager::~ProtocolManager()
{
    Finalize();
}

void ProtocolManager::Initialize(NetworkFactory* networkFactory)
{
    m_networkFactory = networkFactory;
    m_started = 0;
}

Result ProtocolManager::Startup(PacketHandler* packetHandler)
{
    if (packetHandler == 0)
        return static_cast<Result>(0x10c07);
    if (m_started != 0)
        return static_cast<Result>(0x10c08);

    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        protocol->mStationPacketHandler =
            reinterpret_cast<unsigned int>(packetHandler);
    }
    m_started = 1;
    return 0;
}

Result ProtocolManager::StartupProtocols()
{
    if (m_started == 0)
        return static_cast<Result>(0x10c08);

    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        const Result result = protocol->Startup();
        if (result != 0)
        {
            CleanupProtocols();
            return result;
        }
    }
    return 0;
}

Result ProtocolManager::Dispatch()
{
    if (m_started == 0)
        return static_cast<Result>(0x10c08);

    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = static_cast<Protocol*>(node);
        const Result result = protocol->Dispatch();
        if (result != 0)
            return result;
    }
    return 0;
}

void ProtocolManager::CleanupProtocols()
{
    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        protocol->Cleanup();
    }
}

ProtocolId ProtocolManager::CreateProtocolImpl(Protocol* protocol, ProtocolId id)
{
    const unsigned short value = id;
    const unsigned char type = static_cast<unsigned char>(value >> 8);
    if (m_networkFactory == 0 ||
        m_networkFactory->IsSupportedProtocol(type) == 0)
        return ProtocolId(g_piaProtocolDefaultType);
    protocol->SetPort(static_cast<unsigned char>(value));
    m_nodeOffset = static_cast<int>(reinterpret_cast<char*>(
        static_cast<common::ListNode*>(protocol)) - reinterpret_cast<char*>(protocol));
    common::ListNode* sentinel = m_protocols.Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        Protocol* current = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        const unsigned short currentId =
            static_cast<unsigned short>(current->mProtocolId);
        if (currentId >= value)
        {
            if (currentId == value)
                return ProtocolId(g_piaProtocolDefaultType);
            break;
        }
        node = node->next;
    }
    m_protocols.InsertBeforeNode(node, static_cast<common::ListNode*>(protocol));
    if (g_piaProtocolMessageFilteringManager != 0 &&
        protocol->IsProtocolFilteringEnabled() == 0)
    {
        g_piaProtocolMessageFilteringManager->AddNoFilteringProtocolType(type);
    }
    g_piaProtocolTypeMask &= ~__N_29_transport_ProtocolManager_cpp_70a76ebe::getProtocolTypeBit(type);
    return ProtocolId(value);
}

void ProtocolManager::DestroyProtocol(unsigned short id)
{
    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel;
         node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        if (static_cast<unsigned short>(protocol->mProtocolId) != id)
            continue;
        if (g_piaProtocolMessageFilteringManager != 0 &&
            protocol->IsProtocolFilteringEnabled() == 0)
        {
            g_piaProtocolMessageFilteringManager->RemoveNoFilteringProtocolType(
                static_cast<unsigned char>(id >> 8));
        }
        m_protocols.EraseNode(static_cast<common::ListNode*>(protocol));
        DestroyProtocolStorage(protocol);
        return;
    }
}

UnreliableProtocol* ProtocolManager::AllocProtocol(unsigned int size)
{
    pead::Heap* heap = common::HeapManager::GetHeap();
    void* memory = pead::allocateFromSeadHeap(size, heap);
    if (memory != 0)
        memset(memory, 0, size);
    return static_cast<UnreliableProtocol*>(memory);
}

Protocol* ProtocolManager::SearchProtocol(ProtocolId id, unsigned char type)
{
    if ((id >> 8) != type)
        return 0;
    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(reinterpret_cast<char*>(node) - m_nodeOffset);
        if (static_cast<unsigned short>(protocol->mProtocolId) == id)
            return protocol;
    }
    return 0;
}

void ProtocolManager::Finalize()
{
    while (common::ListNode* node = m_protocols.PopBackNode())
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        DestroyProtocolStorage(protocol);
    }
    m_networkFactory = 0;
    m_started = 0;
}

}}}
