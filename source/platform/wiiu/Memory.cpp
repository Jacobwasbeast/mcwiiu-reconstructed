#include "game/core/XMem.hpp"

#include <stdint.h>
#include <cstdlib>
#include <cstring>

#include <coreinit/fastmutex.h>

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

extern "C" uint32_t AllocMutexLock(OSFastMutex* mutex)
{
    OSFastMutex_Lock(mutex);
    return 0;
}

extern "C" uint32_t AllocMutexUnlock(OSFastMutex* mutex)
{
    OSFastMutex_Unlock(mutex);
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
