#include "nn/pia/common/WatermarkManager.hpp"

#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>
#include <string.h>

namespace nn { namespace pia { namespace transport {
common::WatermarkManager* g_piaWatermarkManager = 0;
}}}

namespace nn { namespace pia { namespace common {

namespace
{
void* s_watermarkStorage = 0;
}

WatermarkManager::WatermarkManager()
{
}

WatermarkManager::~WatermarkManager()
{
}

Result WatermarkManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (transport::g_piaWatermarkManager != 0)
        return 0x10c0f;

    pead::Heap* heap = HeapManager::GetHeap();
    s_watermarkStorage = pead::allocateFromSeadHeap(0x410, heap, 8);
    if (s_watermarkStorage == 0)
        return 0x10c01;

    memset(s_watermarkStorage, 0, 0x410);
    transport::g_piaWatermarkManager = new (s_watermarkStorage) WatermarkManager();
    return 0;
}

void WatermarkManager::DestroyInstance()
{
    if (transport::g_piaWatermarkManager != 0)
    {
        transport::g_piaWatermarkManager->~WatermarkManager();
        transport::g_piaWatermarkManager = 0;
    }
    if (s_watermarkStorage != 0)
    {
        pead::getArraySize(s_watermarkStorage);
        pead::freeToSeadHeap(s_watermarkStorage);
        s_watermarkStorage = 0;
    }
}

void WatermarkManager::EnableAllWatermark()
{
    for (unsigned int index = 0; index != cWatermarkNum; ++index)
        m_watermarks[index].m_enabled = true;
}

Watermark* WatermarkManager::GetWatermark(int index)
{
    if (static_cast<unsigned int>(index) >= cWatermarkNum)
        return 0;
    return &m_watermarks[index];
}

}}}
