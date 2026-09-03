#include "vendor/__N_15_FFLiMiiData_cpp_2f9c0036.hpp"

unsigned int FFLiIsNumberCharCode(unsigned short character);
void FFLiCopyWcharT2U16(unsigned short* destination, unsigned int maximumLength, const wchar_t* source);

int __N_15_FFLiMiiData_cpp_2f9c0036::RflToFaceLineAndMake(int value)
{
    return value * 8 + 0x1047325c;
}

unsigned int __N_15_FFLiMiiData_cpp_2f9c0036::VerifyNameFromRFL(unsigned short* name, int length, int maximumNumberCharacters)
{
    int numberCharacters = 0;
    int index = 0;
    while (index < length)
    {
        const unsigned int character = name[index];
        if (character < 0x40)
        {
            if (character == 0)
            {
                return 1;
            }
            if (character == 0x25)
            {
                return 0;
            }
        }
        else
        {
            if (character == 0x40 || character == 0x5c || character == 0xff20)
            {
                return 0;
            }
        }
        if ((character - 0xe000 < 0x1900) ||
            (FFLiIsNumberCharCode(static_cast<unsigned short>(character)) != 0 &&
             ++numberCharacters > maximumNumberCharacters))
        {
            return 0;
        }
        ++index;
    }
    return 1;
}

unsigned int __N_15_FFLiMiiData_cpp_2f9c0036::ReplaceNameFromRFL(unsigned short* name, int length, int maximumNumberCharacters)
{
    if (VerifyNameFromRFL(name, length, maximumNumberCharacters) == 0)
    {
        FFLiCopyWcharT2U16(name, static_cast<unsigned int>(length), L"???");
        return 1;
    }
    return 0;
}

bool __N_15_FFLiMiiData_cpp_2f9c0036::IsBirthPlatformFeature(FFLiBirthPlatform platform)
{
    return platform != FFLiBirthPlatform_None;
}
