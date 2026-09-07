#include <cstdint>

extern "C" void OSFastMutex_Lock();
extern "C" void OSFastMutex_Unlock();

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
