#include "nn/pia/common/Md5Context.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

namespace
{
unsigned int RotateLeft(unsigned int value, unsigned int count)
{
    return (value << count) | (value >> (32 - count));
}

}

void Md5Context::Initialize()
{
    m_state[0] = 0x67452301;
    m_state[1] = 0xefcdab89;
    m_state[2] = 0x98badcfe;
    m_state[3] = 0x10325476;
    m_totalBytes = 0;
}

void Md5Context::Update(const void* data, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    const unsigned int offset = m_totalBytes & 0x3f;
    const unsigned int available = sizeof(m_block) - offset;
    m_totalBytes += size;
    if (size < available)
    {
        if (size != 0)
            memcpy(m_block + offset, bytes, size);
        return;
    }
    memcpy(m_block + offset, bytes, available);
    ProcessBlock();
    bytes += available;
    size -= available;
    while (size >= sizeof(m_block))
    {
        memcpy(m_block, bytes, sizeof(m_block));
        ProcessBlock();
        bytes += sizeof(m_block);
        size -= sizeof(m_block);
    }
    if (size != 0)
        memcpy(m_block, bytes, size);
}

void Md5Context::ProcessBlock()
{
    static const unsigned int shift[64] = {
        7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
        5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
        4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
        6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
    };
    static const unsigned int table[64] = {
        0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
        0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
        0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
        0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
        0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
        0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
        0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
        0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
    };
    unsigned int word[16];
    for (unsigned int i = 0; i < 16; ++i)
        word[i] = static_cast<unsigned int>(m_block[i * 4]) |
                  (static_cast<unsigned int>(m_block[i * 4 + 1]) << 8) |
                  (static_cast<unsigned int>(m_block[i * 4 + 2]) << 16) |
                  (static_cast<unsigned int>(m_block[i * 4 + 3]) << 24);
    unsigned int a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    for (unsigned int i = 0; i < 64; ++i)
    {
        unsigned int f;
        unsigned int index;
        if (i < 16) { f = (b & c) | (~b & d); index = i; }
        else if (i < 32) { f = (d & b) | (~d & c); index = (5 * i + 1) & 15; }
        else if (i < 48) { f = b ^ c ^ d; index = (3 * i + 5) & 15; }
        else { f = c ^ (b | ~d); index = (7 * i) & 15; }
        const unsigned int next = b + RotateLeft(a + f + table[i] + word[index], shift[i]);
        a = d;
        d = c;
        c = b;
        b = next;
    }
    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
}

void Md5Context::GetHash(void* hash)
{
    const unsigned int totalBytes = m_totalBytes;
    static const unsigned char padding = 0x80;
    Update(&padding, 1);
    unsigned int offset = m_totalBytes & 0x3f;
    unsigned int available = sizeof(m_block) - offset;
    if (available < 8)
    {
        memset(m_block + offset, 0, available);
        ProcessBlock();
        offset = 0;
        available = sizeof(m_block);
    }
    if (available > 8)
        memset(m_block + offset, 0, available - 8);
    const unsigned int bitCount = totalBytes << 3;
    m_block[56] = static_cast<unsigned char>(bitCount);
    m_block[57] = static_cast<unsigned char>(bitCount >> 8);
    m_block[58] = static_cast<unsigned char>(bitCount >> 16);
    m_block[59] = static_cast<unsigned char>(bitCount >> 24);
    m_block[60] = 0;
    m_block[61] = 0;
    m_block[62] = 0;
    m_block[63] = 0;
    ProcessBlock();
    unsigned char* output = static_cast<unsigned char*>(hash);
    for (unsigned int i = 0; i < 4; ++i)
    {
        output[i * 4] = static_cast<unsigned char>(m_state[i]);
        output[i * 4 + 1] = static_cast<unsigned char>(m_state[i] >> 8);
        output[i * 4 + 2] = static_cast<unsigned char>(m_state[i] >> 16);
        output[i * 4 + 3] = static_cast<unsigned char>(m_state[i] >> 24);
    }
}

unsigned int Md5Context::GetHashSize() const
{
    return 16;
}

unsigned int Md5Context::GetBlockSize() const
{
    return sizeof(m_block);
}

}}}
