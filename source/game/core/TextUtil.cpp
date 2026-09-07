#include "game/core/TextUtil.hpp"

uint32_t isNewLineChar__Fw(uint16_t character)
{
    return character == 10 || character == 0xD;
}
