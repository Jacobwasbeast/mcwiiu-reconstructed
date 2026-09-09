#ifndef PLATFORM_WIIU_FFLISTOREDATACFL_HPP
#define PLATFORM_WIIU_FFLISTOREDATACFL_HPP

#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

void FFLiPutCRC16(unsigned short seed, void* data, unsigned int size);
bool FFLiIsValidCRC16(const void* data, unsigned int size);
unsigned short FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(unsigned short value);

struct FFLiStoreDataCFL : FFLiMiiDataOfficial
{
    unsigned short m_crc;
    unsigned short m_crc2;

    bool IsValidCRC() const { return FFLiIsValidCRC16(this, 0x60); }
    void SetCRC() { FFLiPutCRC16(0, this, 0x60); }
    void SwapEndian()
    {
        FFLiMiiDataOfficial::SwapEndian();
        m_crc = FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(m_crc);
        m_crc2 = FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(m_crc2);
        SetCRC();
    }
};

#endif
