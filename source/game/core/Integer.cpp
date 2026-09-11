#include "game/core/Integer.hpp"

#include <cwchar>

int Integer::parseInt(const std::wstring& value, int radix)
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&value);
    const wchar_t* text;
    if (7 < *reinterpret_cast<const unsigned int*>(bytes + 0x1c))
        text = *reinterpret_cast<const wchar_t* const*>(bytes + 0x14);
    else
        text = reinterpret_cast<const wchar_t*>(bytes + 4);
    return static_cast<int>(wcstol(text, NULL, radix));
}
