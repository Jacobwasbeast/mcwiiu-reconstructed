#ifndef PLATFORM_WIIU_FFLISTOREDATA_HPP
#define PLATFORM_WIIU_FFLISTOREDATA_HPP

#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

struct FFLiStoreData : FFLiMiiDataOfficial
{
    unsigned short m_crc;
    unsigned short m_crc2;

    void SetCRC();
    void SwapEndian();
    bool IsValidCRC() const;
};

#endif
