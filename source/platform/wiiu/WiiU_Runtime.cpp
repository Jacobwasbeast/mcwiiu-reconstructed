#include <cstdlib>
#include <new>

#include "platform/wiiu/WiiU_Runtime.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

struct OSCalendarTime
{
    int microseconds;
    int milliseconds;
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
    int weekday;
    int yearday;
};

unsigned long long OSGetTime();
OSSystemInfo* OSGetSystemInfo();
void OSTicksToCalendarTime(unsigned long long ticks, OSCalendarTime* calendar);
int FSGetLastErrorCodeForViewer(void* client);

static void* sFileSystemClient;

unsigned int GetLastFileErrorCodeForViewer()
{
    return static_cast<unsigned int>(FSGetLastErrorCodeForViewer(sFileSystemClient));
}

unsigned int GetLastError()
{
    return 0;
}

void GlobalMemoryStatus(MEMORYSTATUS* status)
{
    status->dwTotalPhys = 0;
    status->dwAvailPhys = 0;
    status->dwAvailVirtual = 0;
}

unsigned int GetTickCount()
{
    const unsigned long long microseconds = OSGetTime() * 1000000ULL;
    const unsigned long long ticksPerMicrosecond = OSGetSystemInfo()->busSpeed >> 2;
    return static_cast<unsigned int>(microseconds / ticksPerMicrosecond / 1000ULL);
}

void GetSystemTime(SYSTEMTIME* systemTime)
{
    OSCalendarTime calendar;
    OSTicksToCalendarTime(OSGetTime(), &calendar);
    systemTime->year = static_cast<unsigned short>(calendar.year);
    systemTime->month = static_cast<unsigned short>(calendar.month + 1);
    systemTime->dayOfWeek = static_cast<unsigned short>(calendar.weekday);
    systemTime->day = static_cast<unsigned short>(calendar.day);
    systemTime->hour = static_cast<unsigned short>(calendar.hours);
    systemTime->minute = static_cast<unsigned short>(calendar.minutes);
    systemTime->second = static_cast<unsigned short>(calendar.seconds);
    systemTime->milliseconds = static_cast<unsigned short>(calendar.milliseconds);
}

void GetLocalTime(SYSTEMTIME* systemTime)
{
    GetSystemTime(systemTime);
}

void* operator new(std::size_t size)
{
    if (size == 0)
    {
        size = 1;
    }
    for (;;)
    {
        void* address = std::malloc(size);
        if (address != 0)
        {
            return address;
        }
        std::new_handler handler = std::get_new_handler();
        if (handler == 0)
        {
            return 0;
        }
        handler();
    }
}

void operator delete(void* address) noexcept
{
    if (address != 0)
    {
        std::free(address);
    }
}

void* operator new[](std::size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* address) noexcept
{
    ::operator delete(address);
}
