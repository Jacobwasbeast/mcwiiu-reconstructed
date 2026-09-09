#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"
#include "vendor/__N_29_transport_ProtocolManager_cpp_70a76ebe.hpp"

#include <string.h>

namespace nn { namespace pia { namespace transport {

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

ProtocolId ProtocolManager::CreateProtocolImpl(Protocol* protocol, ProtocolId id)
{
    const unsigned short value = static_cast<unsigned short>(id);
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
    g_piaProtocolTypeMask &=
        ~__N_29_transport_ProtocolManager_cpp_70a76ebe::getProtocolTypeBit(type);
    return ProtocolId(value);
}

void ProtocolManager::DestroyProtocol(ProtocolId id)
{
    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel;
         node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(
            reinterpret_cast<char*>(node) - m_nodeOffset);
        if (static_cast<unsigned short>(protocol->mProtocolId) !=
            static_cast<unsigned short>(id))
            continue;
        if (g_piaProtocolMessageFilteringManager != 0 &&
            protocol->IsProtocolFilteringEnabled() == 0)
        {
            g_piaProtocolMessageFilteringManager->RemoveNoFilteringProtocolType(
                static_cast<unsigned char>(static_cast<unsigned short>(id) >> 8));
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
    if ((static_cast<unsigned short>(id) >> 8) != type)
        return 0;
    common::ListNode* sentinel = m_protocols.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        Protocol* protocol = reinterpret_cast<Protocol*>(reinterpret_cast<char*>(node) - m_nodeOffset);
        if (static_cast<unsigned short>(protocol->mProtocolId) == static_cast<unsigned short>(id))
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
