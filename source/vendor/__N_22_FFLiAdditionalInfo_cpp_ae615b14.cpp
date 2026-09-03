#include "vendor/__N_22_FFLiAdditionalInfo_cpp_ae615b14.hpp"

void __N_22_FFLiAdditionalInfo_cpp_ae615b14::ReplaceInvalidNameChar(unsigned short* name, int length)
{
    for (int index = 0; index < length; ++index)
    {
        unsigned short character = name[index];
        if (character == 0)
            return;
        if (character < 0x20 || character == 0x7f)
            name[index] = 0x3f;
    }
}

bool __N_22_FFLiAdditionalInfo_cpp_ae615b14::IsAvailableFontRegion(FFLFontRegion requestedRegion,
                                                                     FFLFontRegion availableRegion)
{
    return requestedRegion == availableRegion;
}

void __N_22_FFLiAdditionalInfo_cpp_ae615b14::ReplaceNonAsciiNameChar(unsigned short* name, int length)
{
    for (int index = 0; index < length; ++index)
    {
        unsigned short character = name[index];
        if (character == 0)
            return;
        if (character > 0x7f)
            name[index] = 0x3f;
    }
}
