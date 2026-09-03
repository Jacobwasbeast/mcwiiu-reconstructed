#ifndef SOURCE_VENDOR___N_22_FFLIADDITIONALINFO_CPP_AE615B14_HPP
#define SOURCE_VENDOR___N_22_FFLIADDITIONALINFO_CPP_AE615B14_HPP

typedef unsigned int FFLFontRegion;

class __N_22_FFLiAdditionalInfo_cpp_ae615b14
{
public:
    static void ReplaceInvalidNameChar(unsigned short* name, int length);
    static bool IsAvailableFontRegion(FFLFontRegion requestedRegion, FFLFontRegion availableRegion);
    static void ReplaceNonAsciiNameChar(unsigned short* name, int length);
};

#endif
