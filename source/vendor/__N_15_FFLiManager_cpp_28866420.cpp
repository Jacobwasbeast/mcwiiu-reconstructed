#include "vendor/__N_15_FFLiManager_cpp_28866420.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"

#include <new>

class FFLiManager
{
public:
    static bool IsConstruct();
    static int Create(void* work, const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc);
};

void SetModuleId(const char* moduleId);

void SetModuleId(const char*)
{
}

uint32_t __N_15_FFLiManager_cpp_28866420::TempMemorySize(bool temporary)
{
    if (temporary)
    {
        return 0xa0000;
    }
    return 0x520000;
}

FFLiCompressor* __N_15_FFLiManager_cpp_28866420::CreateCompressor(const void* resource, FFLiBufferAllocator* allocator)
{
    if (resource != NULL)
    {
        const unsigned int address = allocator->Allocate(0x1f0, 4);
        if (address != 0)
        {
            FFLiCompressor* compressor = (FFLiCompressor*)(uintptr_t)address;
            return new (compressor) FFLiCompressor(allocator, resource);
        }
    }
    return NULL;
}

int __N_15_FFLiManager_cpp_28866420::InitResImpl(void* work, const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc)
{
    SetModuleId("[SDK+Nintendo:FFL_1_5_6]");
    int result = 0;
    if (!FFLiManager::IsConstruct())
    {
        result = FFLiManager::Create(work, initDesc, resourceDesc);
    }
    return result;
}
