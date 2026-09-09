#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" void OSFastMutex_Lock();
extern "C" void OSFastMutex_Unlock();

enum _XMEMCODEC_TYPE
{
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
};

int XMemCreateCompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**)
{
    return 0;
}

int XMemCreateDecompressionContext(_XMEMCODEC_TYPE, const void*, unsigned int, void**)
{
    return 0;
}

void XMemDestroyCompressionContext(void*) {}
void XMemDestroyDecompressionContext(void*) {}

uint32_t AllocMutexLock()
{
    OSFastMutex_Lock();
    return 0;
}

uint32_t AllocMutexUnlock()
{
    OSFastMutex_Unlock();
    return 0;
}

void* XMemCpy(void* destination, const void* source, unsigned int count)
{
    return std::memcpy(destination, source, count);
}

void* XMemSet(void* destination, int value, unsigned int count)
{
    return std::memset(destination, value, count);
}

void* XMemSet128(void* destination, int value, unsigned int count)
{
    return std::memset(destination, value, count);
}

void* XPhysicalAlloc(unsigned long size, unsigned long, unsigned long, unsigned int)
{
    return std::malloc(size);
}

void XPhysicalFree(void* address)
{
    std::free(address);
}
