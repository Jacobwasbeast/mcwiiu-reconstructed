#include "vendor/__N_16_FFLiCharInfo_cpp_2e82104a.hpp"
#include <cstring>

struct FFLiCharInfo
{
    unsigned char data[288];
};

unsigned int FFLiIsValidCharacterForName(unsigned short character);

unsigned int __N_16_FFLiCharInfo_cpp_2e82104a::VerifyName(const unsigned short* name, int maximumLength)
{
    int index = 0;
    if (maximumLength > 0)
    {
        do
        {
            if (name[index] == 0)
            {
                return 1;
            }
            if (FFLiIsValidCharacterForName(name[index]) == 0)
            {
                return 0;
            }
            ++index;
        }
        while (index < maximumLength);
    }
    return 1;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareParts(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareName(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareCreatorName(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareCreatorID(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareGender(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareBirthMonth(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareBirthDay(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareFavoriteColor(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareHeight(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareBuild(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareFacelineColor(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareMiiVersion(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareCopyable(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareNgWord(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareLocalOnly(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareRegionMove(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareFontRegion(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}

bool __N_16_FFLiCharInfo_cpp_2e82104a::CompareBirthPlatform(const FFLiCharInfo* first, const FFLiCharInfo* second)
{
    return std::memcmp(first, second, sizeof(FFLiCharInfo)) == 0;
}
