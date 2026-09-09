#include "platform/wiiu/FFLiZlibInflator.hpp"
#include "platform/wiiu/FFLiBufferAllocatorForZlib.hpp"
#include <cstring>

FFLiZlibInflator::FFLiZlibInflator(FFLiAllocatorInterface& allocator, int windowBits)
    : mAllocator(allocator), mFinished(false)
{
    std::memset(&mStream, 0, sizeof(mStream));
    mStream.zalloc = (alloc_func)FFLiZlibAllocator::Allocate;
    mStream.zfree = FFLiZlibAllocator::Free;
    mStream.opaque = &mAllocator;
    inflateInit2_(&mStream, windowBits, ZLIB_VERSION, sizeof(mStream));
}

FFLiZlibInflator::~FFLiZlibInflator()
{
    inflateEnd(&mStream);
}

int FFLiZlibInflator::Process(void** output, unsigned int* outputSize,
                              const void** input, unsigned int* inputSize,
                              int flush)
{
    mStream.next_in = (Bytef*)*input;
    mStream.avail_in = *inputSize;
    mStream.next_out = (Bytef*)*output;
    mStream.avail_out = *outputSize;
    const int result = inflate(&mStream, flush);
    if (result == Z_STREAM_END)
    {
        mFinished = true;
    }
    *input = mStream.next_in;
    *inputSize = mStream.avail_in;
    *output = mStream.next_out;
    *outputSize = mStream.avail_out;
    return result;
}

unsigned int FFLiZlibInflator::TotalInputSize() const
{
    return mStream.total_in;
}

unsigned int FFLiZlibInflator::TotalOutputSize() const
{
    return mStream.total_out;
}
