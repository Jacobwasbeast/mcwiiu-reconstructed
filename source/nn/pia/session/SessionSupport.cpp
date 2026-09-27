#include "nn/pia/session/SessionSupport.hpp"

#include <nn/middleware.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
unsigned char s_initialized = 0;
unsigned char s_duringSetup = 0;
}

unsigned char IsInitialized()
{
    return s_initialized;
}

unsigned char IsDuringSetup()
{
    return s_duringSetup;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_SESSION_5_9_0]");
}

Result Initialize()
{
    EmbedModuleInfo();

    if (s_initialized != 0)
        return 0x10c02;

    pead::SafeStringBase<char> heapName("pia session heap");
    common::HeapManager::Setup(static_cast<ModuleType>(7), 0, heapName);
    s_initialized = 1;
    return ModuleStackChecker::Register(static_cast<ModuleType>(7));
}

Result BeginSetup()
{
    if (s_initialized == 0)
        return 0x10c0c;
    if (s_duringSetup != 0)
        return 0x10c08;

    common::HeapManager::SetCurrentHeap(static_cast<ModuleType>(7));
    s_duringSetup = 1;
    return 0;
}

Result EndSetup()
{
    if (s_initialized == 0)
        return 0x10c0c;
    if (s_duringSetup == 0)
        return 0x10c08;

    pead::Heap* heap = common::HeapManager::GetHeap(static_cast<ModuleType>(7));
    if (heap == 0)
        return 0x10c08;

    heap->freeAll();
    s_duringSetup = 0;
    common::HeapManager::ClearCurrentHeap();
    return 0;
}

}}}
