#ifndef PLATFORM_WIIU_PLATFORMTIMING_HPP
#define PLATFORM_WIIU_PLATFORMTIMING_HPP

struct _LARGE_INTEGER
{
    long long QuadPart;
};

unsigned int QueryPerformanceCounter(_LARGE_INTEGER* value);
void QueryPerformanceFrequency(_LARGE_INTEGER* value);

#endif
