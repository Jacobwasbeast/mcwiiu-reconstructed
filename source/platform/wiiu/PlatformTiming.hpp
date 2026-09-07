#ifndef PLATFORM_WIIU_PLATFORMTIMING_HPP
#define PLATFORM_WIIU_PLATFORMTIMING_HPP

struct _LARGE_INTEGER
{
    long long QuadPart;
};

unsigned int QueryPerformanceCounter__FP14_LARGE_INTEGER(_LARGE_INTEGER* value);
void QueryPerformanceFrequency__FP14_LARGE_INTEGER(_LARGE_INTEGER* value);

#endif
