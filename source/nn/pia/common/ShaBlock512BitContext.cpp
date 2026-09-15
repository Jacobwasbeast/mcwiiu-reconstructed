#include "nn/pia/common/ShaBlock512BitContext.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

unsigned int ShaBlock512BitContext::GetBlockSize() const
{
    return sizeof(m_block);
}

void ShaBlock512BitContext::Update(const void* data, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    while (size != 0)
    {
        const unsigned int available = sizeof(m_block) - m_blockSize;
        const unsigned int copied = size < available ? size : available;
        memcpy(m_block + m_blockSize, bytes, copied);
        m_blockSize += copied;
        bytes += copied;
        size -= copied;
        if (m_blockSize == sizeof(m_block))
        {
            ProcessBlock();
            m_blockSize = 0;
            ++m_blockCount;
            if (m_blockCount == 0)
                ++m_blockCountHigh;
        }
    }
    return;
}

void ShaBlock512BitContext::AddPadding()
{
    const unsigned int bitCountHigh = (m_blockCountHigh << 9) | (m_blockCount >> 23);
    const unsigned int bitCountLow = m_blockCount << 9 | (m_blockSize << 3);
    const unsigned char padding[64] = { 0x80 };
    const unsigned int paddingSize = m_blockSize < 56 ? 56 : 120;
    unsigned char length[8];
    length[0] = static_cast<unsigned char>(bitCountHigh >> 24);
    length[1] = static_cast<unsigned char>(bitCountHigh >> 16);
    length[2] = static_cast<unsigned char>(bitCountHigh >> 8);
    length[3] = static_cast<unsigned char>(bitCountHigh);
    length[4] = static_cast<unsigned char>(bitCountLow >> 24);
    length[5] = static_cast<unsigned char>(bitCountLow >> 16);
    length[6] = static_cast<unsigned char>(bitCountLow >> 8);
    length[7] = static_cast<unsigned char>(bitCountLow);
    Update(padding, paddingSize - m_blockSize);
    Update(length, sizeof(length));
}

void ShaBlock512BitContext::ProcessBlock()
{
}

}}}
