#include "vendor/__N_21_FFLiPartsTextures_cpp_6d55e3e1.hpp"

#include <cstring>

void FFLiInvalidateTexture(_GX2Texture* texture);

void __N_21_FFLiPartsTextures_cpp_6d55e3e1::ExpressionToEyeUseFlag(bool* useFlags,
                                                                     unsigned int expression)
{
    static const unsigned int eyeFlagOffsets[19][2] =
    {
        { 0, 0 }, { 1, 1 }, { 0, 0 }, { 2, 2 }, { 3, 3 }, { 4, 4 },
        { 0, 0 }, { 1, 1 }, { 0, 0 }, { 2, 2 }, { 3, 3 }, { 4, 4 },
        { 5, 0 }, { 0, 5 }, { 5, 0 }, { 0, 5 }, { 5, 0 }, { 0, 5 },
        { 5, 5 }
    };
    std::memset(useFlags, 0, 6);
    for (unsigned int index = 0; index < 19; ++index)
    {
        if ((expression & (1U << index)) != 0)
        {
            useFlags[eyeFlagOffsets[index][0]] = true;
            useFlags[eyeFlagOffsets[index][1]] = true;
        }
    }
}

void __N_21_FFLiPartsTextures_cpp_6d55e3e1::ExpressionToMouthUseFlag(bool* useFlags,
                                                                       unsigned int expression)
{
    static const unsigned int mouthFlagOffsets[19] =
    {
        0, 0, 1, 2, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 3, 3, 4, 4, 2
    };
    std::memset(useFlags, 0, 5);
    for (unsigned int index = 0; index < 19; ++index)
    {
        if ((expression & (1U << index)) != 0)
            useFlags[mouthFlagOffsets[index]] = true;
    }
}

void __N_21_FFLiPartsTextures_cpp_6d55e3e1::InvalidateTextures(_GX2Texture** textures,
                                                                unsigned int count)
{
    for (unsigned int index = 0; index < count; ++index)
    {
        if (textures[index] != 0)
            FFLiInvalidateTexture(textures[index]);
    }
}
