#include "platform/wiiu/FFLiDefaultParams.hpp"

#include "platform/wiiu/FFLiDefaultParams_sLight.inc"

#include "platform/wiiu/FFLiDefaultParams_sRim.inc"

#include "platform/wiiu/FFLiDefaultParams_sMaterial.inc"

const unsigned char* FFLiDefaultParams::GetMaterial(int32_t drawType)
{
    return sMaterial + drawType * 56;
}

const unsigned char* FFLiDefaultParams::GetLight()
{
    return sLight;
}

const unsigned char* FFLiDefaultParams::GetRim()
{
    return sRim;
}
