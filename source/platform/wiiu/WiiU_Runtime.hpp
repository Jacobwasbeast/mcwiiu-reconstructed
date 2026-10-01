#ifndef WIIU_RUNTIME_HPP
#define WIIU_RUNTIME_HPP

#include <coreinit/filesystem.h>

#ifndef ERROR_CANCELLED
#define ERROR_CANCELLED 1223
#endif

struct FILETIME
{
    unsigned long low;
    unsigned long high;
};

struct MEMORYSTATUS
{
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
};

struct _MEMORYSTATUS
{
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
};

struct SYSTEMTIME
{
    unsigned short year;
    unsigned short month;
    unsigned short dayOfWeek;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milliseconds;
};

unsigned int GetLastFileErrorCodeForViewer();
unsigned int GetLastError();
void GlobalMemoryStatus(MEMORYSTATUS* status);
void GlobalMemoryStatus(_MEMORYSTATUS* status);
unsigned int GetTickCount();
void GetSystemTime(SYSTEMTIME* systemTime);
void GetLocalTime(SYSTEMTIME* systemTime);
unsigned int FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* systemTime);

FSClient* GetWiiUFileSystemClient();
extern "C" void WiiUInitialise();

#endif
