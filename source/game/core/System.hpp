#ifndef GAME_CORE_SYSTEM_HPP
#define GAME_CORE_SYSTEM_HPP

#include <string.h>

#include "game/world/biome/arrayWithLength.hpp"

class System
{
public:
    enum ETimeGranularity
    {
        eTimeGranularity_MilliSecs,
        eTimeGranularity_MicroSecs,
        eTimeGranularity_NanoSecs
    };

    template <class T>
    static void arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T>* dest, unsigned int destPos, unsigned int length);

    static long long processTime(ETimeGranularity granularity);
    static long long processTimeInSeconds();
    static long long processTimeInMilliSecs();
    static long long processTimeInMicroSecs();
    static long long processTimeInNanoSecs();

    static long long currentRealTimeMillis();
    static long long currentUnixEpochTimeSeconds();

    static void ReverseUSHORT(unsigned short* value);
    static void ReverseSHORT(short* value);
    static void ReverseULONG(unsigned int* value);
    static void ReverseULONG(unsigned long* value);
    static void ReverseINT(int* value);
    static void ReverseULONGLONG(long long* value);
    static void ReverseCHAR16A(wchar_t* text, int count);
};

template <class T>
void System::arraycopy(arrayWithLength<T> src, unsigned int srcPos, arrayWithLength<T>* dest, unsigned int destPos, unsigned int length)
{
    memcpy(dest->data + destPos, src.data + srcPos, length * sizeof(T));
}

#endif
