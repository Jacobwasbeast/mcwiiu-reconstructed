#include "nn/pia/chat/N_12_chat_Api_cpp_c724e756.hpp"

#include "nn/pia/chat/CodecFacade.hpp"

namespace nn { namespace pia { namespace chat {

void __N_12_chat_Api_cpp_c724e756::createSystemSingletons()
{
    CodecFacade::CreateInstance();
    if (g_codecFacade != 0)
        g_codecFacade->Initialize();
}

void __N_12_chat_Api_cpp_c724e756::destroySystemSingletons()
{
    if (g_codecFacade != 0)
    {
        g_codecFacade->Finalize();
        CodecFacade::DestroyInstance();
    }
}

}}}
