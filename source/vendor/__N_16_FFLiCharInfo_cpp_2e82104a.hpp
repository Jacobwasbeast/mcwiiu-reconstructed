#ifndef SOURCE_VENDOR___N_16_FFLICHARINFO_CPP_2E82104A_HPP
#define SOURCE_VENDOR___N_16_FFLICHARINFO_CPP_2E82104A_HPP

struct FFLiCharInfo;

struct __N_16_FFLiCharInfo_cpp_2e82104a
{
    static unsigned int VerifyName(const unsigned short* name, int maximumLength);
    static bool CompareParts(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareName(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareCreatorName(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareCreatorID(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareGender(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareBirthMonth(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareBirthDay(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareFavoriteColor(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareHeight(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareBuild(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareFacelineColor(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareMiiVersion(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareCopyable(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareNgWord(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareLocalOnly(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareRegionMove(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareFontRegion(const FFLiCharInfo* first, const FFLiCharInfo* second);
    static bool CompareBirthPlatform(const FFLiCharInfo* first, const FFLiCharInfo* second);
};

#endif
