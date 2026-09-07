#include "platform/wiiu/PlatformTiming.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

unsigned long long OSGetTime();
OSSystemInfo* OSGetSystemInfo();

unsigned int QueryPerformanceCounter__FP14_LARGE_INTEGER(_LARGE_INTEGER* value)
{
    const unsigned long long microseconds = OSGetTime() * 1000000ULL;
    const unsigned long long ticksPerMicrosecond = OSGetSystemInfo()->busSpeed >> 2;
    value->QuadPart = static_cast<long long>(microseconds / ticksPerMicrosecond);
    return 1;
}

void QueryPerformanceFrequency__FP14_LARGE_INTEGER(_LARGE_INTEGER* value)
{
    value->QuadPart = 1000000;
}
