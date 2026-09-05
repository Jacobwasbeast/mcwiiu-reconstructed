#ifndef PLATFORM_WIIU_FFLIMIIDATAOFFICIAL_HPP
#define PLATFORM_WIIU_FFLIMIIDATAOFFICIAL_HPP

#include "platform/wiiu/FFLiMiiDataCore.hpp"

struct FFLiMiiDataOfficial : FFLiMiiDataCore
{
    unsigned short dataWords[10];

    void SwapEndian();
};

#endif
