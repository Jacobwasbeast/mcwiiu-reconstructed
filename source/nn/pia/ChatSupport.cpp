#include "nn/pia/ChatSupport.hpp"
#include "nn/pia/chat/CodecFacade.hpp"

extern void SetModuleId(const char* moduleId);

namespace nn
{
namespace pia
{
namespace chat
{

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
