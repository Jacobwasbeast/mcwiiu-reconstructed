#include "game/world/block/AbstractProperty.hpp"

#include <string>

int javaStringHashCode(const std::wstring& value)
{
    int hash = 0;
    for (std::wstring::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        hash = hash * 31 + static_cast<unsigned short>(*it);
    }
    return hash;
}
