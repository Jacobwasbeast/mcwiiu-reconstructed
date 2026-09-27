#include "nn/pia/lan/LanSupport.hpp"

#include <nn/middleware.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"

namespace
{
unsigned char g_lanInitialized = 0;
unsigned char g_lanInSetup = 0;
}

namespace nn { namespace pia { namespace lan {

Result Initialize()
{
    EmbedModuleInfo();

    if (g_lanInitialized != 0)
        return 0x10c02;

    pead::SafeStringBase<char> heapName("pia lan heap");
    common::HeapManager::Setup(static_cast<ModuleType>(0x10), 0, heapName);
    g_lanInitialized = 1;
    return ModuleStackChecker::Register(static_cast<ModuleType>(0x10));
}

Result BeginSetup()
{
    if (g_lanInitialized == 0)
        return 0x10c0c;
    if (g_lanInSetup != 0)
        return 0x10c08;

    common::HeapManager::SetCurrentHeap(static_cast<ModuleType>(0x10));
    g_lanInSetup = 1;
    return 0;
}

Result EndSetup()
{
    if (g_lanInitialized == 0)
        return 0x10c0c;
    if (g_lanInSetup == 0)
        return 0x10c08;

    pead::Heap* heap = common::HeapManager::GetHeap(static_cast<ModuleType>(0x10));
    if (heap == 0)
        return 0x10c08;

    heap->freeAll();
    g_lanInSetup = 0;
    common::HeapManager::ClearCurrentHeap();
    return 0;
}

void Finalize()
{
    if (g_lanInitialized == 0)
        return;

    common::HeapManager::Cleanup(static_cast<ModuleType>(0x10));
    g_lanInitialized = 0;
    ModuleStackChecker::Unregister(static_cast<ModuleType>(0x10));
}

unsigned char IsInitialized()
{
    return g_lanInitialized;
}

unsigned char IsInSetupMode()
{
    return g_lanInSetup;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_LAN_5_9_0]");
}

}}}
