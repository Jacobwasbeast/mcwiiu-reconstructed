#include "game/util/LegacyUtilityFunctions.hpp"
#include "vendor/MilesRib.hpp"

extern "C" void AIL_debug_log(const char* format, ...);
extern "C" void AIL_set_error(const char* pError);
extern "C" unsigned int RIB_find_file_provider(const char* pInterfaceName, const char* pAttributeName,
                                               const char* pFileName);

static const char ASI_CODEC_INTERFACE[] = "ASI codec";
static const char PROVIDER_PROPERTY_ENTRY[] = "PROVIDER_property";
static const char OUT_OF_MEMORY_ERROR[] = "Out of memory.";
static const unsigned int RIB_FUNCTION = 0;

extern "C" void outresfloat()
{
    AIL_debug_log("!-Result = %f");
}

extern "C" void outresint64(uint32_t low, uint32_t high)
{
    AIL_debug_log("!-Result = %llu", high, low, high);
}

extern "C" void outresuint(uint32_t value)
{
    AIL_debug_log("!-Result = %ld", value);
}

int32_t ourtoupper(char value)
{
    return (value >= 'a' && value <= 'z') ? static_cast<char>(value - 0x20) : value;
}

int OnlyNum(const char* value)
{
    const char* current = value;
    while (*current != '\0')
    {
        if ((current != value || *current != '-') &&
            (static_cast<unsigned int>(static_cast<char>(*current + static_cast<unsigned char>(-'0'))) >= 10 && *current != '.'))
        {
            return 0;
        }
        ++current;
    }
    return 1;
}

extern "C" int AIL_stricmp(const char* first, const char* second)
{
    for (;;)
    {
        const int firstChar = ourtoupper(*first);
        const int secondChar = ourtoupper(*second);
        if (secondChar < firstChar)
        {
            return 1;
        }
        if (firstChar < secondChar)
        {
            return -1;
        }
        if (firstChar == 0)
        {
            return 0;
        }
        ++first;
        ++second;
    }
}

extern "C" int AIL_strnicmp(const char* first, const char* second, int count)
{
    for (; count != 0; --count)
    {
        const int firstChar = ourtoupper(*first);
        const int secondChar = ourtoupper(*second);
        if (firstChar < secondChar)
        {
            return -1;
        }
        if (secondChar < firstChar)
        {
            return 1;
        }
        if (firstChar == 0)
        {
            return 0;
        }
        ++first;
        ++second;
    }
    return 0;
}

extern "C" unsigned int find_ASI_provider(const char* attributeName, const char* fileName)
{
    unsigned int provider = RIB_find_file_provider(ASI_CODEC_INTERFACE, attributeName, fileName);
    if (provider != 0)
    {
        unsigned int propertyToken = 0;
        RIB_INTERFACE_ENTRY entry;
        entry.uType = RIB_FUNCTION;
        entry.pEntryName = PROVIDER_PROPERTY_ENTRY;
        entry.pToken = &propertyToken;
        entry.uSubType = 0;
        if (RIB_request_interface(provider, ASI_CODEC_INTERFACE, 1, &entry) != 0)
        {
            AIL_set_error(OUT_OF_MEMORY_ERROR);
            provider = 0;
        }
    }
    return provider;
}
