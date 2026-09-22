#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"
#include "vendor/__N_14_common_Api_cpp_61832df6.hpp"

extern void SetModuleId(const char* moduleId);

namespace nn
{
namespace pia
{
void EmbedMiddlewareInfo();
}
}

namespace
{
__N_14_common_Api_cpp_61832df6 g_commonApi;
unsigned char g_commonInitialized = 0;
unsigned char g_commonInSetup = 0;
}

namespace nn
{
namespace pia
{
namespace common
{

Result Initialize(void* workMemory, unsigned int workMemorySize)
{
    nn::pia::EmbedMiddlewareInfo();
    EmbedModuleInfo();

    if (workMemory == 0 || workMemorySize == 0 || (workMemorySize & 3) != 0)
        return 0x10c07;
    if (g_commonInitialized != 0)
        return 0x10c02;

    pead::SafeStringBase<char> heapName("pia common heap");
    HeapManager::Setup(static_cast<ModuleType>(2), workMemorySize, heapName);
    pead::CoreInfo::configure();
    g_commonApi.registerInternalPrintCallback();
    g_commonInitialized = 1;
    ModuleStackChecker::Register(static_cast<ModuleType>(2));
    return 0;
}

Result BeginSetup()
{
    if (g_commonInitialized == 0)
        return 0x10c0c;
    if (g_commonInSetup != 0)
        return 0x10c08;

    HeapManager::SetCurrentHeap(static_cast<ModuleType>(2));
    g_commonInSetup = 1;
    g_commonApi.createSystemSingletons();
    return 0;
}

Result EndSetup()
{
    if (g_commonInitialized == 0)
        return 0x10c0c;
    if (g_commonInSetup == 0)
        return 0x10c08;

    pead::Heap* heap = HeapManager::GetHeap();
    if (heap != 0)
        heap->freeAll();
    HeapManager::ClearCurrentHeap();
    g_commonInSetup = 0;
    return 0;
}

void Finalize()
{
    if (g_commonInitialized == 0)
        return;

    if (g_commonInSetup != 0)
        EndSetup();
    g_commonApi.unregisterInternalPrintCallback();
    g_commonApi.destroySystemSingletons();
    HeapManager::Cleanup(static_cast<ModuleType>(2));
    HeapManager::ClearCurrentHeap();
    g_commonInitialized = 0;
    ModuleStackChecker::Unregister(static_cast<ModuleType>(2));
}

unsigned char IsInitialized()
{
    return g_commonInitialized;
}

unsigned char IsInSetupMode()
{
    return g_commonInSetup;
}

void serializeU16(unsigned char* buffer, unsigned short value)
{
    buffer[0] = static_cast<unsigned char>(value >> 8);
    buffer[1] = static_cast<unsigned char>(value);
}

void serializeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>(value >> 16);
    buffer[2] = static_cast<unsigned char>(value >> 8);
    buffer[3] = static_cast<unsigned char>(value);
}

unsigned short deserializeU16(const unsigned char* buffer)
{
    return static_cast<unsigned short>((static_cast<unsigned int>(buffer[0]) << 8) | buffer[1]);
}

unsigned int deserializeU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) |
           (static_cast<unsigned int>(buffer[1]) << 16) |
           (static_cast<unsigned int>(buffer[2]) << 8) | buffer[3];
}

}
}
}

