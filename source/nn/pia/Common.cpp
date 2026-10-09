#include "nn/pia/Common.hpp"

#include <nn/middleware.h>
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/ModuleStackChecker.hpp"
#include "platform/wiiu/pead.hpp"
#include "vendor/__N_14_common_Api_cpp_61832df6.hpp"

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

    HeapManager::Initialize(workMemory, workMemorySize);
    pead::SafeStringBase<char> heapName("pia common heap");
    HeapManager::Setup(cModuleType_Common, 0, heapName);
    pead::CoreInfo::configure();
    g_commonApi.registerInternalPrintCallback();
    g_commonInitialized = 1;
    ModuleStackChecker::Register(cModuleType_Common);
    return 0;
}

Result BeginSetup()
{
    if (g_commonInitialized == 0)
        return 0x10c0c;
    if (g_commonInSetup != 0)
        return 0x10c08;

    HeapManager::SetCurrentHeap(cModuleType_Common);
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

    HeapManager::GetHeap()->adjust();
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
    HeapManager::Cleanup(cModuleType_Common);
    HeapManager::Finalize();
    g_commonInitialized = 0;
    ModuleStackChecker::Unregister(cModuleType_Common);
}

unsigned char IsInitialized()
{
    return g_commonInitialized;
}

unsigned char IsInSetupMode()
{
    return g_commonInSetup;
}

void serializeU64(unsigned char* buffer, unsigned long long value)
{
    buffer[0] = static_cast<unsigned char>((value >> 56) & 0xff);
    buffer[1] = static_cast<unsigned char>((value >> 48) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 40) & 0xff);
    buffer[3] = static_cast<unsigned char>((value >> 32) & 0xff);
    buffer[4] = static_cast<unsigned char>((value >> 24) & 0xff);
    buffer[5] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[6] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[7] = static_cast<unsigned char>(value & 0xff);
}

unsigned long long deserializeU64(const unsigned char* buffer)
{
    unsigned long long value = 0;
    value |= static_cast<unsigned long long>(buffer[0]) << 56;
    value |= static_cast<unsigned long long>(buffer[1]) << 48;
    value |= static_cast<unsigned long long>(buffer[2]) << 40;
    value |= static_cast<unsigned long long>(buffer[3]) << 32;
    value |= static_cast<unsigned long long>(buffer[4]) << 24;
    value |= static_cast<unsigned long long>(buffer[5]) << 16;
    value |= static_cast<unsigned long long>(buffer[6]) << 8;
    value |= static_cast<unsigned long long>(buffer[7]);
    return value;
}

void serializeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>(value >> 16);
    buffer[2] = static_cast<unsigned char>(value >> 8);
    buffer[3] = static_cast<unsigned char>(value);
}

unsigned int deserializeU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) |
           (static_cast<unsigned int>(buffer[1]) << 16) |
           (static_cast<unsigned int>(buffer[2]) << 8) | buffer[3];
}

void serializeU16(unsigned char* buffer, unsigned short value)
{
    buffer[0] = static_cast<unsigned char>(value >> 8);
    buffer[1] = static_cast<unsigned char>(value);
}

unsigned short deserializeU16(const unsigned char* buffer)
{
    return static_cast<unsigned short>((static_cast<unsigned int>(buffer[0]) << 8) | buffer[1]);
}

void serializeU8(unsigned char* buffer, unsigned char value)
{
    buffer[0] = value;
}

unsigned char deserializeU8(const unsigned char* buffer)
{
    return buffer[0];
}

}
}
}

