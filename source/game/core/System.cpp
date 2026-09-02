#include "game/core/System.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

long long OSGetTime();
OSSystemInfo* OSGetSystemInfo();

long long System::processTime(ETimeGranularity granularity)
{
    switch (granularity)
    {
    case eTimeGranularity_MilliSecs:
        return (unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busSpeed / 4);

    case eTimeGranularity_MicroSecs:
        return (unsigned long long)OSGetTime() * 1000000 / (OSGetSystemInfo()->busSpeed / 4);

    case eTimeGranularity_NanoSecs:
        return (unsigned long long)OSGetTime() * 32000 / ((unsigned long long)(OSGetSystemInfo()->busSpeed / 4) / 31250);
    }

    return 0;
}

long long System::processTimeInSeconds()
{
    return processTime(eTimeGranularity_MilliSecs) / 1000;
}

long long System::processTimeInMilliSecs()
{
    return processTime(eTimeGranularity_MilliSecs);
}

long long System::processTimeInNanoSecs()
{
    return processTime(eTimeGranularity_NanoSecs);
}

long long System::currentRealTimeMillis()
{
    return (unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busSpeed / 4);
}

long long System::currentUnixEpochTimeSeconds()
{
    return currentRealTimeMillis() / 1000;
}

void System::ReverseUSHORT(unsigned short* value)
{
    unsigned short original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;

    destination[0] = source[1];
    destination[1] = source[0];
}

void System::ReverseSHORT(short* value)
{
    short original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;

    destination[0] = source[1];
    destination[1] = source[0];
}

void System::ReverseULONG(unsigned int* value)
{
    unsigned int original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;

    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseINT(int* value)
{
    int original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;

    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseULONGLONG(long long* value)
{
    long long original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;

    destination[0] = source[7];
    destination[1] = source[6];
    destination[2] = source[5];
    destination[3] = source[4];
    destination[4] = source[3];
    destination[5] = source[2];
    destination[6] = source[1];
    destination[7] = source[0];
}

void System::ReverseCHAR16A(wchar_t* text, int count)
{
    for (int i = 0; i < count; i++)
    {
        ReverseUSHORT((unsigned short*)&text[i]);
    }
}
