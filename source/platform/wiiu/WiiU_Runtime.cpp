#include <cstdlib>
#include <new>

#include <coreinit/filesystem.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

static FSClient* sFileSystemClient = 0;

extern void altMain();
extern void altMainStop();

FSClient* GetWiiUFileSystemClient()
{
    return sFileSystemClient;
}

unsigned int FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* systemTime)
{
    (void)fileTime;
    (void)systemTime;
    return 0;
}

static void stateChangeCallback(FSClient* client, FSVolumeState state, void*)
{
    if (state == FS_VOLUME_STATE_READY)
    {
        altMainStop();
    }
    else
    {
        (void)FSGetLastErrorCodeForViewer(client);
    }
}

extern "C" void WiiUInitialise()
{
    FSInit();
    sFileSystemClient = static_cast<FSClient*>(std::malloc(0x1700));
    if (sFileSystemClient != 0)
    {
        FSAddClient(sFileSystemClient, FS_ERROR_FLAG_NONE);
        FSStateChangeParams notification = {stateChangeCallback, 0, 0};
        FSSetStateChangeNotification(sFileSystemClient, &notification);
        WiiUFileCacheInitialise();
        altMainStop();
    }
}

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

void GlobalMemoryStatus(_MEMORYSTATUS* status)
{
    status->dwTotalPhys = 0;
    status->dwAvailPhys = 0;
    status->dwAvailVirtual = 0;
}

unsigned int GetTickCount()
{
    const unsigned long long microseconds = OSGetTime() * 1000000ULL;
    const unsigned long long ticksPerMicrosecond = OSGetSystemInfo()->busClockSpeed >> 2;
    return static_cast<unsigned int>(microseconds / ticksPerMicrosecond / 1000ULL);
}

void GetSystemTime(SYSTEMTIME* systemTime)
{
    OSCalendarTime calendar;
    OSTicksToCalendarTime(OSGetTime(), &calendar);
    systemTime->year = static_cast<unsigned short>(calendar.tm_year);
    systemTime->month = static_cast<unsigned short>(calendar.tm_mon + 1);
    systemTime->dayOfWeek = static_cast<unsigned short>(calendar.tm_wday);
    systemTime->day = static_cast<unsigned short>(calendar.tm_mday);
    systemTime->hour = static_cast<unsigned short>(calendar.tm_hour);
    systemTime->minute = static_cast<unsigned short>(calendar.tm_min);
    systemTime->second = static_cast<unsigned short>(calendar.tm_sec);
    systemTime->milliseconds = static_cast<unsigned short>(calendar.tm_msec);
}

void GetLocalTime(SYSTEMTIME* systemTime)
{
    GetSystemTime(systemTime);
}

#if !defined(RECON_NATIVE)
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
#endif
