#include "nn/pia/common/ZlibCompressor.hpp"

namespace nn { namespace pia { namespace common {

ZlibCompressor::ZlibCompressor()
    : m_workMemory(0),
      m_workMemorySize(0),
      m_allocationCount(0),
      m_allocationOffset(0),
      m_deflatableSize(0)
{
}

ZlibCompressor::~ZlibCompressor()
{
}

bool ZlibCompressor::IsInitialized() const
{
    return m_workMemory != 0;
}

Result ZlibCompressor::Initialize(void* buffer, unsigned int size)
{
    if (buffer == 0 || (reinterpret_cast<uintptr_t>(buffer) & 3u) != 0 || size < 0x16c4u)
    {
        return static_cast<Result>(0x10c07);
    }

    if (IsInitialized())
    {
        return static_cast<Result>(0x10c02);
    }

    m_workMemory = static_cast<unsigned char*>(buffer);
    m_workMemorySize = size;
    return 0;
}

void ZlibCompressor::Finalize()
{
    m_workMemory = 0;
    m_workMemorySize = 0;
}

void* ZlibCompressor::myAlloc(void* opaque, unsigned int items, unsigned int size)
{
    ZlibCompressor* compressor = static_cast<ZlibCompressor*>(opaque);
    if (compressor->m_allocationCount < 5)
    {
        unsigned int oldOffset = compressor->m_allocationOffset;
        const int64_t signedProduct =
            static_cast<int64_t>(static_cast<int32_t>(items)) *
            static_cast<int64_t>(static_cast<int32_t>(size));
        const unsigned int nextOffset = oldOffset + static_cast<unsigned int>(signedProduct);
        if (nextOffset <= compressor->m_workMemorySize)
        {
            compressor->m_allocationOffset = nextOffset;
            ++compressor->m_allocationCount;
            return compressor->m_workMemory + oldOffset;
        }
    }
    return 0;
}

void ZlibCompressor::myFree(void* opaque, void*)
{
    ZlibCompressor* compressor = static_cast<ZlibCompressor*>(opaque);
    if (compressor->m_allocationCount > 0)
    {
        --compressor->m_allocationCount;
    }
}

Result ZlibCompressor::Startup(unsigned char* output, unsigned int outputSize,
                               int level, int windowBits, int memLevel)
{
    if (output == 0 || outputSize == 0 ||
        static_cast<unsigned int>(level) >= 10u ||
        (static_cast<unsigned int>(windowBits) - 8u) >= 8u ||
        (static_cast<unsigned int>(memLevel) - 1u) >= 9u)
    {
        return static_cast<Result>(0x10c07);
    }

    if (!IsInitialized())
    {
        return static_cast<Result>(0x10c08);
    }

    m_stream.zalloc = &ZlibCompressor::myAlloc;
    m_stream.zfree = &ZlibCompressor::myFree;
    m_stream.opaque = this;

    const int initResult = deflateInit2_(&m_stream, level, Z_DEFLATED,
                                         windowBits, memLevel, Z_DEFAULT_STRATEGY,
                                         ZLIB_VERSION, static_cast<int>(sizeof(z_stream)));
    if (initResult != Z_OK)
    {
        return initResult == Z_MEM_ERROR ? static_cast<Result>(0x10c01)
                                         : static_cast<Result>(0xc0b);
    }

    const uLong bound = deflateBound(&m_stream, 0x80u);
    m_stream.next_out = output;
    m_stream.avail_out = outputSize;
    m_deflatableSize = static_cast<unsigned int>(bound - 0x80u);
    return 0;
}

void ZlibCompressor::Cleanup()
{
    deflateEnd(&m_stream);
    m_allocationOffset = 0;
}

Result ZlibCompressor::Deflate(const unsigned char* input, unsigned int inputSize)
{
    if (input == 0 || inputSize == 0)
    {
        return static_cast<Result>(0x10c07);
    }

    if (m_stream.avail_out < inputSize + m_deflatableSize + 6u)
    {
        return static_cast<Result>(0x10c04);
    }

    m_stream.next_in = const_cast<Bytef*>(input);
    m_stream.avail_in = inputSize;
    if (deflate(&m_stream, Z_SYNC_FLUSH) != Z_OK || m_stream.avail_in != 0)
    {
        return static_cast<Result>(0xc0b);
    }
    return 0;
}

Result ZlibCompressor::FinishDeflate(unsigned int* outputSize)
{
    if (outputSize == 0)
    {
        return static_cast<Result>(0x10c07);
    }

    if (m_stream.total_out == 0 || m_stream.avail_in != 0)
    {
        return static_cast<Result>(0x10c08);
    }

    if (deflate(&m_stream, Z_FINISH) != Z_STREAM_END)
    {
        return static_cast<Result>(0xc0b);
    }

    *outputSize = static_cast<unsigned int>(m_stream.total_out);
    return 0;
}

}}}
