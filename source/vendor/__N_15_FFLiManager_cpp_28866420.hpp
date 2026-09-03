#ifndef SOURCE_VENDOR___N_15_FFLIMANAGER_CPP_28866420_HPP
#define SOURCE_VENDOR___N_15_FFLIMANAGER_CPP_28866420_HPP

#include <stdint.h>

class FFLiBufferAllocator;
class FFLiCompressor;
struct FFLInitDesc;
struct FFLResourceDesc;

class __N_15_FFLiManager_cpp_28866420
{
public:
    static uint32_t TempMemorySize(bool temporary);
    static FFLiCompressor* CreateCompressor(const void* resource, FFLiBufferAllocator* allocator);
    static int InitResImpl(void* work, const FFLInitDesc* initDesc, const FFLResourceDesc* resourceDesc);
};

#endif
