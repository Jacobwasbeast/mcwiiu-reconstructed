#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace common
{

PayloadSizeManager* g_payloadSizeManager = 0;

namespace
{
void* g_payloadSizeManagerStorage = 0;
}

PayloadSizeManager::PayloadSizeManager()
    : m_mtuSize(0x5c0),
      m_minMtuSize(0xffffffffU),
      m_maxMtuSize(0),
      m_field0c(0),
      m_signatureSize(0)
{
}

PayloadSizeManager::~PayloadSizeManager()
{
}

Result PayloadSizeManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (g_payloadSizeManager != 0)
        return 0x10c0f;

    pead::Heap* heap = HeapManager::GetHeap();
    g_payloadSizeManagerStorage = pead::allocateFromSeadHeap(
        sizeof(PayloadSizeManager), heap, 8);
    if (g_payloadSizeManagerStorage == 0)
        return 0x10c01;

    g_payloadSizeManager = new (g_payloadSizeManagerStorage) PayloadSizeManager();
    return 0;
}

void PayloadSizeManager::DestroyInstance()
{
    if (g_payloadSizeManager != 0)
    {
        g_payloadSizeManager->~PayloadSizeManager();
        g_payloadSizeManager = 0;
    }
    if (g_payloadSizeManagerStorage != 0)
    {
        pead::getArraySize(g_payloadSizeManagerStorage);
        pead::freeToSeadHeap(g_payloadSizeManagerStorage);
        g_payloadSizeManagerStorage = 0;
    }
}

Result PayloadSizeManager::SetMtuSize(unsigned int mtuSize)
{
    if (mtuSize < m_minMtuSize || mtuSize > m_maxMtuSize)
        return 0x10c07;
    m_mtuSize = mtuSize;
    return 0;
}

}
}
}
