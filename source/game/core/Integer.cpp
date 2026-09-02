#include "game/core/Integer.hpp"

#include <cwchar>

int Integer::parseInt(const std::wstring& value, int radix)
{
    return std::wcstol(value.c_str(), NULL, radix);
}
