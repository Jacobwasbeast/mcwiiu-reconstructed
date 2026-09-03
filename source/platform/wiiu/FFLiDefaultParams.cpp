#include "platform/wiiu/FFLiDefaultParams.hpp"

int32_t FFLiDefaultParams::GetMaterial(int32_t drawType)
{
    return drawType * 56 + 0x10474160;
}

uint32_t FFLiDefaultParams::GetLight()
{
    return 0x1047410C;
}

uint32_t FFLiDefaultParams::GetRim()
{
    return 0x1047414C;
}
