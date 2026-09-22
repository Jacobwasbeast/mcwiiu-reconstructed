#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>
#include "nn/pia/transport/TransportSupport.hpp"

namespace nn { namespace pia { namespace transport {



ProtocolMessageFilteringManager* g_piaProtocolMessageFilteringManager = 0;

namespace
{
void* g_filteringManagerStorage = 0;
}

ProtocolMessageFilteringManager::ProtocolMessageFilteringManager()
    : m_types(), m_count(0)
{
}

ProtocolMessageFilteringManager::~ProtocolMessageFilteringManager()
{
}

unsigned int ProtocolMessageFilteringManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (g_piaProtocolMessageFilteringManager != 0)
        return 0x10c0f;

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_filteringManagerStorage = pead::allocateFromSeadHeap(0x24, heap, 8);
    if (g_filteringManagerStorage == 0)
        return 0x10c01;
    g_piaProtocolMessageFilteringManager =
        new (g_filteringManagerStorage) ProtocolMessageFilteringManager();
    return 0;
}

void ProtocolMessageFilteringManager::DestroyInstance()
{
    if (g_piaProtocolMessageFilteringManager != 0)
    {
        g_piaProtocolMessageFilteringManager->~ProtocolMessageFilteringManager();
        g_piaProtocolMessageFilteringManager = 0;
    }
    if (g_filteringManagerStorage != 0)
    {
        pead::freeToSeadHeap(g_filteringManagerStorage);
        g_filteringManagerStorage = 0;
    }
}

unsigned int ProtocolMessageFilteringManager::IsFilteringEnabled(
    unsigned char protocolType) const
{
    int remaining = m_count;
    int index = 0;
    while (remaining > 0)
    {
        if (m_types[index] == protocolType)
            return 0;
        ++index;
        --remaining;
    }
    return 1;
}

unsigned int ProtocolMessageFilteringManager::AddNoFilteringProtocolType(
    unsigned char protocolType)
{
    if (m_count > 0x1f)
        return 0;
    const int index = m_count;
    ++m_count;
    m_types[index] = protocolType;
    return 1;
}

unsigned int ProtocolMessageFilteringManager::RemoveNoFilteringProtocolType(
    unsigned char protocolType)
{
    const int count = m_count;
    if (count == 0)
        return 0;
    int index = 0;
    while (index < count && m_types[index] != protocolType)
        ++index;
    if (index == count)
        return 0;
    for (int i = index; i < count - 1; ++i)
        m_types[i] = m_types[i + 1];
    m_count = count - 1;
    return 1;
}


}}}
