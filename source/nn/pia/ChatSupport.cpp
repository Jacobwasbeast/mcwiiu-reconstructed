#include "nn/pia/ChatSupport.hpp"
#include <new>

extern void SetModuleId(const char* moduleId);

namespace nn
{
namespace pia
{
namespace chat
{

CodecFacade* g_codecFacade = 0;
void* g_codecStorage = 0;

CodecFacade::CodecFacade()
    : m_state(0)
{
}

CodecFacade::~CodecFacade()
{
}

void CodecFacade::Trace(unsigned long long) const
{
}

void CodecFacade::Initialize()
{
    m_state = 0;
}

void CodecFacade::Finalize()
{
}

void CodecFacade::CreateInstance()
{
    if (IsInitialized() == 0 || IsInSetupMode() == 0)
        return;
    if (g_codecFacade != 0)
        return;

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_codecStorage = pead::allocateFromSeadHeap(8, heap, 8);
    if (g_codecStorage == 0)
        return;
    g_codecFacade = new (g_codecStorage) CodecFacade();
}

void CodecFacade::DestroyInstance()
{
    if (g_codecFacade != 0)
    {
        g_codecFacade->Finalize();
        g_codecFacade->~CodecFacade();
        g_codecFacade = 0;
    }
    if (g_codecStorage != 0)
    {
        pead::freeToSeadHeap(g_codecStorage);
        g_codecStorage = 0;
    }
}

void ChatApi::createSystemSingletons()
{
    CodecFacade::CreateInstance();
    if (g_codecFacade != 0)
        g_codecFacade->Initialize();
}

void ChatApi::destroySystemSingletons()
{
    CodecFacade::DestroyInstance();
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_CHAT_5_9_0]");
}

}
}
}

namespace nn
{
namespace pia
{
namespace common
{
void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_COMMON_5_9_0]");
}
}
}
}
