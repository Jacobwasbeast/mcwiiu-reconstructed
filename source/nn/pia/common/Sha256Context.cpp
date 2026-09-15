#include "nn/pia/common/Sha256Context.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

namespace
{
unsigned int RotateRight(unsigned int value, unsigned int count)
{
    return (value >> count) | (value << (32 - count));
}
}

void Sha256Context::Initialize()
{
    memset(m_block, 0, sizeof(m_block));
    m_blockSize = 0;
    m_blockCount = 0;
    m_blockCountHigh = 0;
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
}

void Sha256Context::Update(const void* data, unsigned int size)
{
    ShaBlock512BitContext::Update(data, size);
}

void Sha256Context::ProcessBlock()
{
    static const unsigned int roundConstants[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    unsigned int words[64];
    for (unsigned int i = 0; i < 16; ++i)
        words[i] = static_cast<unsigned int>(m_block[i * 4]) << 24 | static_cast<unsigned int>(m_block[i * 4 + 1]) << 16 | static_cast<unsigned int>(m_block[i * 4 + 2]) << 8 | m_block[i * 4 + 3];
    for (unsigned int i = 16; i < 64; ++i)
    {
        const unsigned int s0 = RotateRight(words[i - 15], 7) ^ RotateRight(words[i - 15], 18) ^ (words[i - 15] >> 3);
        const unsigned int s1 = RotateRight(words[i - 2], 17) ^ RotateRight(words[i - 2], 19) ^ (words[i - 2] >> 10);
        words[i] = words[i - 16] + s0 + words[i - 7] + s1;
    }
    unsigned int a=m_state[0],b=m_state[1],c=m_state[2],d=m_state[3],e=m_state[4],f=m_state[5],g=m_state[6],h=m_state[7];
    for (unsigned int i = 0; i < 64; ++i)
    {
        const unsigned int s1 = RotateRight(e, 6) ^ RotateRight(e, 11) ^ RotateRight(e, 25);
        const unsigned int choose = (e & f) ^ (~e & g);
        const unsigned int workingRound = h + s1 + choose + roundConstants[i] + words[i];
        const unsigned int s0 = RotateRight(a, 2) ^ RotateRight(a, 13) ^ RotateRight(a, 22);
        const unsigned int majority = (a & b) ^ (a & c) ^ (b & c);
        const unsigned int workingState = s0 + majority;
        h=g; g=f; f=e; e=d+workingRound; d=c; c=b; b=a; a=workingRound+workingState;
    }
    m_state[0]+=a; m_state[1]+=b; m_state[2]+=c; m_state[3]+=d; m_state[4]+=e; m_state[5]+=f; m_state[6]+=g; m_state[7]+=h;
}

void Sha256Context::GetHash(void* hash)
{
    AddPadding();
    unsigned char* output = static_cast<unsigned char*>(hash);
    for (unsigned int i = 0; i < 8; ++i)
    {
        output[i * 4] = static_cast<unsigned char>(m_state[i] >> 24);
        output[i * 4 + 1] = static_cast<unsigned char>(m_state[i] >> 16);
        output[i * 4 + 2] = static_cast<unsigned char>(m_state[i] >> 8);
        output[i * 4 + 3] = static_cast<unsigned char>(m_state[i]);
    }
}

unsigned int Sha256Context::GetHashSize() const
{
    return 32;
}

}}}
