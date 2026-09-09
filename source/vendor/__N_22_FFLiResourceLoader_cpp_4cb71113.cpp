#include "vendor/__N_22_FFLiResourceLoader_cpp_4cb71113.hpp"
#include "platform/wiiu/FFLiBufferAllocatorForZlib.hpp"
#include "platform/wiiu/FFLiZlibInflator.hpp"
#include <stdint.h>

extern uint8_t FFLiResourceWindowBitsToZlibWindowBits__F22FFLiResourceWindowBits(uint32_t value);

uint8_t FFLiResourceWindowBitsToZlibWindowBits(FFLiResourceWindowBits windowBits)
{
    return FFLiResourceWindowBitsToZlibWindowBits__F22FFLiResourceWindowBits(static_cast<uint32_t>(windowBits));
}

bool __N_22_FFLiResourceLoader_cpp_4cb71113::Uncompress(
    void* destination, const void* source, FFLiResourceUncompressBuffer* buffer,
    const FFLiResourcePartsInfo& parts)
{
    FFLiBufferAllocatorForZlib allocator((void*)buffer->TempBuffer(), buffer->TempBufferSize());
    FFLiZlibInflator inflator(allocator,
        FFLiResourceWindowBitsToZlibWindowBits(static_cast<FFLiResourceWindowBits>(parts.windowBits)));
    void* output = destination;
    const void* input = source;
    unsigned int outputSize = parts.uncompressedSize;
    unsigned int inputSize = parts.compressedSize;
    return inflator.Process(&output, &outputSize, &input, &inputSize, 4) == 1;
}

bool FFLiResourceLoaderUncompress(void* destination, const void* source,
                                  FFLiResourceUncompressBuffer* buffer,
                                  const FFLiResourcePartsInfo& parts)
{
    return __N_22_FFLiResourceLoader_cpp_4cb71113::Uncompress(destination, source, buffer, parts);
}
