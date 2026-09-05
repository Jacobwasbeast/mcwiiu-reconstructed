#ifndef PLATFORM_WIIU_FFLIMIIDATACORE_HPP
#define PLATFORM_WIIU_FFLIMIIDATACORE_HPP

#include "platform/wiiu/FFLiTypes.hpp"

struct FFLiMiiDataCore
{
    unsigned char header[12];
    FFLCreateID createId;
    FFLiAuthorID authorId;
    unsigned char data[40];

    void SwapEndian();
};

#endif
