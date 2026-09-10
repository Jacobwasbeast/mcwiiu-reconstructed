#ifndef PLATFORM_WIIU_CAFE_THREAD_LOCAL_HPP
#define PLATFORM_WIIU_CAFE_THREAD_LOCAL_HPP

#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <map>

namespace CafeThreadLocal {

inline OSMutex& mutex()
{
    static OSMutex value;
    static bool initialized = false;
    if (!initialized)
    {
        OSInitMutex(&value);
        initialized = true;
    }
    return value;
}

template<typename T>
T& slot(std::map<OSThread*, T>& values)
{
    OSThread* thread = OSGetCurrentThread();
    OSLockMutex(&mutex());
    T& entry = values[thread];
    OSUnlockMutex(&mutex());
    return entry;
}

}

#endif
