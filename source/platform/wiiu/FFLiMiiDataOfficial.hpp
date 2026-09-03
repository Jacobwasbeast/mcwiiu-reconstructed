#ifndef PLATFORM_WIIU_FFLIMIIDATAOFFICIAL_HPP
#define PLATFORM_WIIU_FFLIMIIDATAOFFICIAL_HPP

struct FFLiMiiDataCore
{
    unsigned char header[12];
    unsigned char createId[10];
    unsigned char authorId[10];
    unsigned char data[40];

    void SwapEndian();
};

struct FFLiMiiDataOfficial : FFLiMiiDataCore
{
    unsigned short dataWords[10];

    void SwapEndian();
};

#endif
