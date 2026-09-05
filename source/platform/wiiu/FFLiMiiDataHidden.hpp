#ifndef PLATFORM_WIIU_FFLIMIIDATAHIDDEN_HPP
#define PLATFORM_WIIU_FFLIMIIDATAHIDDEN_HPP

struct FFLiMiiDataHidden
{
    unsigned char data[64];

    void SwapEndian();
};

#endif
