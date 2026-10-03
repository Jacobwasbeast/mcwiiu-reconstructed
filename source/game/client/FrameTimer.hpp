#ifndef LEGACY_CONSOLE_EDITION_SOURCE_GAME_CLIENT_FRAMETIMER_HPP
#define LEGACY_CONSOLE_EDITION_SOURCE_GAME_CLIENT_FRAMETIMER_HPP

#include <cstddef>
#include <stdint.h>

struct BcfStream
{
    uint8_t reserved0[0x18];
    void* readContext;
    int (*read)(void*, void*, uint32_t, int);
    uint8_t reserved20[0x10];
    int32_t frameState;
    uint8_t reserved34[0x0c];
    uint32_t scanStart;
    uint8_t reserved44[0x14];
    uint8_t* buffer;
    uint32_t bufferLength;
    uint32_t pendingBytes;
    uint32_t bufferCapacity;
};

void FindFrameBoundary(int streamAddress);

#endif
