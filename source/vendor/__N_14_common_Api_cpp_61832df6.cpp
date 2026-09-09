#include "vendor/__N_14_common_Api_cpp_61832df6.hpp"
#include "nn/pia/PayloadSizeManager.hpp"

namespace pead
{
void freeToSeadHeap(void* pointer);
unsigned int getArraySize(void* pointer);
namespace system
{
void PrintStringImpl(const char* text, int level);
}
}

namespace
{
int g_printCallbackInstalled;
__N_14_common_Api_cpp_61832df6::DelegatePrint g_delegatePrint;
pead::IDelegate1<const pead::PrintConfig::PrintEventArg&>* g_registeredPrintCallback;
void (*g_printFunction)(const char*, int);
}

void __N_14_common_Api_cpp_61832df6::registerInternalPrintCallback()
{
    if (g_printCallbackInstalled == 0)
    {
        g_printCallbackInstalled = 1;
        g_registeredPrintCallback = &g_delegatePrint;
    }
    pead::PrintConfig::registerFinalCallback(g_registeredPrintCallback);
}

void __N_14_common_Api_cpp_61832df6::unregisterInternalPrintCallback()
{
    pead::PrintConfig::registerFinalCallback(0);
}

void __N_14_common_Api_cpp_61832df6::destroySystemSingletons()
{
    nn::pia::common::PayloadSizeManager::DestroyInstance();
}

void __N_14_common_Api_cpp_61832df6::createSystemSingletons()
{
    nn::pia::common::PayloadSizeManager::CreateInstance();
}

void __N_14_common_Api_cpp_61832df6::DelegatePrint::invoke(const pead::PrintConfig::PrintEventArg& event)
{
    if (g_printFunction == 0)
    {
        pead::system::PrintStringImpl(event.text, event.level);
        return;
    }
    g_printFunction(event.text, event.level);
}

void __N_14_common_Api_cpp_61832df6::MyAllocCallback::invoke(const pead::HeapMgr::AllocCallbackArg*)
{
}

void __N_14_common_Api_cpp_61832df6::MyFreeCallback::invoke(const pead::HeapMgr::FreeCallbackArg*)
{
}
