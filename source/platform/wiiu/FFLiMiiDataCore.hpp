#ifndef PLATFORM_WIIU_FFLIMIIDATACORE_HPP
#define PLATFORM_WIIU_FFLIMIIDATACORE_HPP

struct FFLiMiiDataCore
{
    unsigned char header[12];
    unsigned char createId[10];
    unsigned char authorId[10];
    unsigned char data[40];

    void SwapEndian();
};

#endif
