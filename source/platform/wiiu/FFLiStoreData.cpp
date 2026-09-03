#include "platform/wiiu/FFLiStoreData.hpp"

void FFLiPutCRC16(unsigned short seed, void* data, unsigned int size);
bool FFLiIsValidCRC16(const void* data, unsigned int size);
unsigned short FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(unsigned short value);

void FFLiStoreData::SetCRC()
{
    FFLiPutCRC16(0, this, 0x60);
}

void FFLiStoreData::SwapEndian()
{
    FFLiMiiDataOfficial::SwapEndian();
    m_crc = FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(m_crc);
    m_crc2 = FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(m_crc2);
    SetCRC();
}

bool FFLiStoreData::IsValidCRC() const
{
    return FFLiIsValidCRC16(this, 0x60);
}
