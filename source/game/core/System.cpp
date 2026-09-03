#include "game/core/System.hpp"

struct OSSystemInfo
{
    unsigned int busSpeed;
};

long long OSGetTime();
OSSystemInfo* OSGetSystemInfo();

void System::arraycopy(arrayWithLength<unsigned char> src, unsigned int srcPos, arrayWithLength<unsigned char>* dest, unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length);
}

void System::arraycopy(arrayWithLength<Node*> src, unsigned int srcPos, arrayWithLength<Node*>* dest, unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(Node*));
}

void System::arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos, arrayWithLength<Biome*>* dest, unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(Biome*));
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int>* dest, unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(int));
}

void System::arraycopy(arrayWithLength<unsigned short> src, unsigned int srcPos, arrayWithLength<unsigned short>* dest, unsigned int destPos, unsigned int length)
{
    std::memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(unsigned short));
}

unsigned int System::processTime(ETimeGranularity granularity)
{
    switch (granularity)
    {
    case eTimeGranularity_MilliSecs:
        return (unsigned int)((unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busSpeed / 4));
    case eTimeGranularity_MicroSecs:
        return (unsigned int)((unsigned long long)OSGetTime() * 1000000 / (OSGetSystemInfo()->busSpeed / 4));
    case eTimeGranularity_NanoSecs:
        return (unsigned int)((unsigned long long)OSGetTime() * 32000 / ((unsigned long long)(OSGetSystemInfo()->busSpeed / 4) / 31250));
    }
    return 0;
}

uint64_t System::processTimeInSeconds()
{
    return processTime(eTimeGranularity_MilliSecs) / 1000;
}

unsigned int System::processTimeInMilliSecs()
{
    return processTime(eTimeGranularity_MilliSecs);
}

unsigned int System::processTimeInMicroSecs()
{
    return processTime(eTimeGranularity_MicroSecs);
}

unsigned int System::processTimeInNanoSecs()
{
    return processTime(eTimeGranularity_NanoSecs);
}

uint64_t System::currentRealTimeMillis()
{
    return (unsigned long long)OSGetTime() * 1000 / (OSGetSystemInfo()->busSpeed / 4);
}

uint64_t System::currentUnixEpochTimeSeconds()
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
    unsigned short original = (unsigned short)*value;
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

void System::ReverseULONG(unsigned long* value)
{
    unsigned long original = *value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseINT(int* value)
{
    unsigned int original = (unsigned int)*value;
    unsigned char* source = (unsigned char*)&original;
    unsigned char* destination = (unsigned char*)value;
    destination[0] = source[3];
    destination[1] = source[2];
    destination[2] = source[1];
    destination[3] = source[0];
}

void System::ReverseULONGLONG(long long* value)
{
    unsigned long long original = (unsigned long long)*value;
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
