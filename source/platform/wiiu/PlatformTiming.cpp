#include "platform/wiiu/PlatformTiming.hpp"

#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

unsigned int QueryPerformanceCounter(_LARGE_INTEGER* value)
{
    const unsigned long long microseconds = OSGetTime() * 1000000ULL;
    const unsigned long long ticksPerMicrosecond = OSGetSystemInfo()->busClockSpeed >> 2;
    value->QuadPart = static_cast<long long>(microseconds / ticksPerMicrosecond);
    return 1;
}

void QueryPerformanceFrequency(_LARGE_INTEGER* value)
{
    value->QuadPart = 1000000;
}
