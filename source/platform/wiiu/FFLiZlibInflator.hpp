#ifndef PLATFORM_WIIU_FFLIZLIBINFLATOR_HPP
#define PLATFORM_WIIU_FFLIZLIBINFLATOR_HPP

#include "platform/wiiu/FFLiAllocatorInterface.hpp"
#include "platform/wiiu/FFLiZlibAllocator.hpp"
#include <zlib.h>

class FFLiZlibInflator
{
public:
    FFLiZlibInflator(FFLiAllocatorInterface& allocator, int windowBits);
    FFLiZlibInflator(FFLiAllocatorInterface& allocator);
    virtual ~FFLiZlibInflator();

    virtual void Process(void** output, unsigned int* outputSize, const void** input,
                         unsigned int* inputSize, int flush);
    virtual unsigned int TotalInputSize() const;
    virtual unsigned int TotalOutputSize() const;

    static bool Inflate(void* output, unsigned int* outputSize, const void* input,
                        unsigned int inputSize, void* workspace,
                        unsigned int workspaceSize, int windowBits);

private:
    FFLiZlibAllocator mAllocator;
    z_stream mStream;
    bool mFinished;
};

#endif
