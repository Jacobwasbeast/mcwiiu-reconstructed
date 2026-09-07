#include "game/data/BitStorage.hpp"

#include <cstdint>

int32_t getMaskInt__FUiT1(uint32_t offset, uint32_t width)
{
    return static_cast<int32_t>((0xffffffffU >> ((0x20U - width) & 0x3f)) << (offset & 0x3f));
}
