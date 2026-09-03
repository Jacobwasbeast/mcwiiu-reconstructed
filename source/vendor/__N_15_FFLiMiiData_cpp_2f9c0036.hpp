#ifndef SOURCE_VENDOR___N_15_FFLIMIIDATA_CPP_2F9C0036_HPP
#define SOURCE_VENDOR___N_15_FFLIMIIDATA_CPP_2F9C0036_HPP

enum FFLiBirthPlatform
{
    FFLiBirthPlatform_None = 0,
    FFLiBirthPlatform_Wii = 1,
    FFLiBirthPlatform_WiiU = 2,
    FFLiBirthPlatform_3DS = 3
};

struct __N_15_FFLiMiiData_cpp_2f9c0036
{
    static int RflToFaceLineAndMake(int value);
    static unsigned int VerifyNameFromRFL(unsigned short* name, int length, int maximumNumberCharacters);
    static unsigned int ReplaceNameFromRFL(unsigned short* name, int length, int maximumNumberCharacters);
    static bool IsBirthPlatformFeature(FFLiBirthPlatform platform);
};

#endif
