#ifndef WIIU_RUNTIME_HPP
#define WIIU_RUNTIME_HPP

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
unsigned int GetTickCount();
void GetSystemTime(SYSTEMTIME* systemTime);
void GetLocalTime(SYSTEMTIME* systemTime);

#endif
