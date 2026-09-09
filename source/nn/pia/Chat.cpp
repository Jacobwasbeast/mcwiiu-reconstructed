#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/ChatSupport.hpp"
#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace chat
{

namespace
{
unsigned char g_chatInitialized = 0;
unsigned char g_chatInSetup = 0;
}

Result Initialize()
{
    EmbedModuleInfo();
    if (!common::IsInitialized())
        return 0x10c08;
    if (g_chatInitialized != 0)
        return 0x10c02;

    pead::SafeStringBase<char> heapName("pia_chat_heap");
    const ModuleType chatModule = static_cast<ModuleType>(0xb);
    common::HeapManager::Setup(chatModule, 0, heapName);
    g_chatInitialized = 1;
    ModuleStackChecker::Register(chatModule);
    return 0;
}

Result BeginSetup()
{
    if (g_chatInitialized == 0)
        return 0x10c0c;
    if (g_chatInSetup != 0)
        return 0x10c08;

    common::HeapManager::SetCurrentHeap(static_cast<ModuleType>(0xb));
    g_chatInSetup = 1;
    ChatApi::createSystemSingletons();
    return 0;
}

Result EndSetup()
{
    if (g_chatInitialized == 0)
        return 0x10c0c;
    if (g_chatInSetup == 0)
        return 0x10c08;

    pead::Heap* heap = common::HeapManager::GetHeap();
    if (heap != 0)
        heap->freeAll();
    common::HeapManager::ClearCurrentHeap();
    g_chatInSetup = 0;
    return 0;
}

Result Finalize()
{
    if (g_chatInitialized == 0)
        return 0;
    if (g_chatInSetup != 0)
        EndSetup();

    ChatApi::destroySystemSingletons();
    common::HeapManager::Cleanup(static_cast<ModuleType>(0xb));
    g_chatInitialized = 0;
    ModuleStackChecker::Unregister(static_cast<ModuleType>(0xb));
    return 0;
}

unsigned char IsInitialized()
{
    return g_chatInitialized;
}

unsigned char IsInSetupMode()
{
    return g_chatInSetup;
}

}

void Assert(const char*, const char*, unsigned int, const char*, ...)
{
}

}
}
