#include "nn/pia/common/AesImpl.hpp"
#include "nn/pia/common/common.hpp"

#include <cstring>

namespace nn { namespace pia { namespace common {

namespace
{
unsigned char Multiply(unsigned char left, unsigned char right)
{
    unsigned char result = 0;
    while (right != 0)
    {
        if ((right & 1) != 0)
            result ^= left;
        left = static_cast<unsigned char>((left << 1) ^ ((left & 0x80) != 0 ? 0x1b : 0));
        right = static_cast<unsigned char>(right >> 1);
    }
    return result;
}

unsigned char Substitute(unsigned char value)
{
    unsigned char inverse = value;
    if (value != 0)
    {
        inverse = 1;
        for (unsigned int index = 0; index < 254; ++index)
            inverse = Multiply(inverse, value);
    }
    return static_cast<unsigned char>(inverse ^
        static_cast<unsigned char>((inverse << 1) | (inverse >> 7)) ^
        static_cast<unsigned char>((inverse << 2) | (inverse >> 6)) ^
        static_cast<unsigned char>((inverse << 3) | (inverse >> 5)) ^
        static_cast<unsigned char>((inverse << 4) | (inverse >> 4)) ^ 0x63);
}

uint32_t SubstituteWord(uint32_t word)
{
    return (static_cast<uint32_t>(Substitute(static_cast<unsigned char>(word >> 24))) << 24) |
           (static_cast<uint32_t>(Substitute(static_cast<unsigned char>(word >> 16))) << 16) |
           (static_cast<uint32_t>(Substitute(static_cast<unsigned char>(word >> 8))) << 8) |
           Substitute(static_cast<unsigned char>(word));
}

uint32_t RotateWord(uint32_t word)
{
    return (word << 8) | (word >> 24);
}

void AddRoundKey(unsigned char* state, const uint32_t* roundKeys)
{
    for (unsigned int index = 0; index < 16; ++index)
        state[index] ^= static_cast<unsigned char>(
            roundKeys[index / 4] >> (24 - (index % 4) * 8));
}

void ShiftRows(unsigned char* state)
{
    unsigned char shifted[16];
    for (unsigned int column = 0; column < 4; ++column)
        for (unsigned int row = 0; row < 4; ++row)
            shifted[column * 4 + row] = state[((column + row) & 3) * 4 + row];
    std::memcpy(state, shifted, sizeof(shifted));
}

void MixColumns(unsigned char* state)
{
    for (unsigned int column = 0; column < 4; ++column)
    {
        unsigned char* item = state + column * 4;
        const unsigned char first = item[0];
        const unsigned char second = item[1];
        const unsigned char third = item[2];
        const unsigned char fourth = item[3];
        item[0] = Multiply(first, 2) ^ Multiply(second, 3) ^ third ^ fourth;
        item[1] = first ^ Multiply(second, 2) ^ Multiply(third, 3) ^ fourth;
        item[2] = first ^ second ^ Multiply(third, 2) ^ Multiply(fourth, 3);
        item[3] = Multiply(first, 3) ^ second ^ third ^ Multiply(fourth, 2);
    }
}

uint32_t InverseMixWord(uint32_t word)
{
    const unsigned char first = static_cast<unsigned char>(word >> 24);
    const unsigned char second = static_cast<unsigned char>(word >> 16);
    const unsigned char third = static_cast<unsigned char>(word >> 8);
    const unsigned char fourth = static_cast<unsigned char>(word);
    return (static_cast<uint32_t>(Multiply(first, 14) ^ Multiply(second, 11) ^
                                  Multiply(third, 13) ^ Multiply(fourth, 9)) << 24) |
           (static_cast<uint32_t>(Multiply(first, 9) ^ Multiply(second, 14) ^
                                  Multiply(third, 11) ^ Multiply(fourth, 13)) << 16) |
           (static_cast<uint32_t>(Multiply(first, 13) ^ Multiply(second, 9) ^
                                  Multiply(third, 14) ^ Multiply(fourth, 11)) << 8) |
           (Multiply(first, 11) ^ Multiply(second, 13) ^
            Multiply(third, 9) ^ Multiply(fourth, 14));
}
}

template <>
AesImpl<XCUiL_2_16>::~AesImpl()
{
    std::memset(m_roundKeys, 0, sizeof(m_roundKeys));
}

template <>
void AesImpl<XCUiL_2_16>::Initialize(const void* key, unsigned int length, bool encrypt)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(key);
    const unsigned int words = length / sizeof(uint32_t);
    for (unsigned int index = 0; index < words && index < 44; ++index)
    {
        m_roundKeys[index] = (static_cast<uint32_t>(bytes[index * 4]) << 24) |
            (static_cast<uint32_t>(bytes[index * 4 + 1]) << 16) |
            (static_cast<uint32_t>(bytes[index * 4 + 2]) << 8) |
            bytes[index * 4 + 3];
    }

    static const unsigned char roundConstants[] =
        { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };
    for (unsigned int index = words; index < 44; ++index)
    {
        uint32_t prior = m_roundKeys[index - 1];
        if ((index % words) == 0)
            prior = SubstituteWord(RotateWord(prior)) ^
                (static_cast<uint32_t>(roundConstants[index / words - 1]) << 24);
        else if (words > 6 && (index % words) == 4)
            prior = SubstituteWord(prior);
        m_roundKeys[index] = m_roundKeys[index - words] ^ prior;
    }

    if (!encrypt)
        for (unsigned int round = 1; round < 10; ++round)
            for (unsigned int index = 0; index < 4; ++index)
                m_roundKeys[round * 4 + index] =
                    InverseMixWord(m_roundKeys[round * 4 + index]);
}

template <>
void AesImpl<XCUiL_2_16>::EncryptBlock(void* destination, unsigned int,
                                       const void* source, unsigned int) const
{
    unsigned char state[16];
    std::memcpy(state, source, sizeof(state));
    AddRoundKey(state, m_roundKeys);
    for (unsigned int round = 1; round < 10; ++round)
    {
        for (unsigned int index = 0; index < 16; ++index)
            state[index] = Substitute(state[index]);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, m_roundKeys + round * 4);
    }
    for (unsigned int index = 0; index < 16; ++index)
        state[index] = Substitute(state[index]);
    ShiftRows(state);
    AddRoundKey(state, m_roundKeys + 40);
    std::memcpy(destination, state, sizeof(state));
}

}}}
