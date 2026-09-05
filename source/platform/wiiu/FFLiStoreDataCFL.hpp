#ifndef PLATFORM_WIIU_FFLISTOREDATACFL_HPP
#define PLATFORM_WIIU_FFLISTOREDATACFL_HPP

#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

struct FFLiStoreDataCFL : FFLiMiiDataOfficial
{
    unsigned short m_crc;
    unsigned short m_crc2;

    bool IsValidCRC() const;
    void SetCRC();
    void SwapEndian();
};

#endif
