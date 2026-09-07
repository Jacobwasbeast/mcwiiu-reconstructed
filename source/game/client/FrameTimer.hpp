#ifndef LEGACY_CONSOLE_EDITION_SOURCE_GAME_CLIENT_FRAMETIMER_HPP
#define LEGACY_CONSOLE_EDITION_SOURCE_GAME_CLIENT_FRAMETIMER_HPP

#include <cstddef>
#include <cstdint>

struct BcfStream
{
    std::uint8_t reserved0[0x18];
    void* readContext;
    int (*read)(void*, void*, std::uint32_t, int);
    std::uint8_t reserved20[0x10];
    std::int32_t frameState;
    std::uint8_t reserved34[0x0c];
    std::uint32_t scanStart;
    std::uint8_t reserved44[0x14];
    std::uint8_t* buffer;
    std::uint32_t bufferLength;
    std::uint32_t pendingBytes;
    std::uint32_t bufferCapacity;
};

void FindFrameBoundary__Fi(BcfStream* stream);

#endif
