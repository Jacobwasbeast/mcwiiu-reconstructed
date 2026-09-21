#include "nn/pia/common/common.hpp"

#include <string.h>

namespace {

inline uint32_t LoadBigWord(const void* address)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(address);
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           static_cast<uint32_t>(bytes[3]);
}

inline void StoreBigWord(void* address, uint32_t value)
{
    uint8_t* bytes = static_cast<uint8_t*>(address);
    bytes[0] = static_cast<uint8_t>(value >> 24);
    bytes[1] = static_cast<uint8_t>(value >> 16);
    bytes[2] = static_cast<uint8_t>(value >> 8);
    bytes[3] = static_cast<uint8_t>(value);
}

}

namespace nn { namespace pia { namespace common {

void GcmModeImpl<GcmModeGenericHelper>::BlockCallback(void* output,
                                                       const void* input,
                                                       const void* context)
{
    const AesImpl<XCUiL_2_16>* cipher =
        static_cast<const AesImpl<XCUiL_2_16>*>(context);
    cipher->EncryptBlock(output, 0x10, input, 0x10);
}

void GcmModeImpl<GcmModeGenericHelper>::Initialize(
    const AesImpl<XCUiL_2_16>& helper)
{
    m_cipher = &helper;
    m_encryptBlock = &BlockCallback;
    InitializeHashKey();
    m_state = 1;
}

GcmModeImpl<GcmModeGenericHelper>::~GcmModeImpl()
{
    if (this != 0)
        memset(this, 0, 0x180);
}

void GcmModeImpl<GcmModeGenericHelper>::GfMultH(Block* block)
{
    static const uint32_t reduction[16] = {
        0x00000000u, 0x1c200000u, 0x38400000u, 0x24600000u,
        0x70800000u, 0x6ca00000u, 0x48c00000u, 0x54e00000u,
        0xe1000000u, 0xfd200000u, 0xd9400000u, 0xc5600000u,
        0x91800000u, 0x8da00000u, 0xa9c00000u, 0xb5e00000u
    };
    uint32_t state0 = LoadBigWord(&m_hashKeyWords[0]);
    uint32_t state1 = LoadBigWord(&m_hashKeyWords[1]);
    uint32_t state2 = LoadBigWord(&m_hashKeyWords[2]);
    uint32_t state3 = LoadBigWord(&m_hashKeyWords[3]);
    unsigned int remaining = sizeof(block->bytes);
    unsigned int reverseIndex = sizeof(block->bytes) - 1;
    while (remaining != 0)
    {
        const uint8_t value = block->bytes[reverseIndex];
        const unsigned int highNibble = static_cast<unsigned int>(value >> 4);
        const unsigned int lowNibble =
            static_cast<unsigned int>(value & 0x0fu);
        const uint32_t low0 = LoadBigWord(&m_reductionTable[lowNibble][0]);
        const uint32_t low1 = LoadBigWord(&m_reductionTable[lowNibble][1]);
        const uint32_t low2 = LoadBigWord(&m_reductionTable[lowNibble][2]);
        const uint32_t low3 = LoadBigWord(&m_reductionTable[lowNibble][3]);
        uint32_t value0;
        uint32_t value1;
        uint32_t value2;
        uint32_t value3;

        if (remaining == sizeof(block->bytes))
        {
            value0 = low0;
            value1 = low1;
            value2 = low2;
            value3 = low3;
            const uint32_t rotated1 = value0 >> 4;
            const uint32_t rotated2 = (value2 << 28) | (value3 >> 4);
            const uint32_t rotated3 = (value0 << 28) | (value1 >> 4);
            state0 = rotated1;
            state1 = rotated3;
            state2 = (value2 >> 4) | (value1 << 28);
            state3 = rotated2;
            state0 ^= reduction[value3 & 0x0fu];
            state0 ^= LoadBigWord(&m_reductionTable[highNibble][0]);
            state1 = rotated3 ^ LoadBigWord(&m_reductionTable[highNibble][1]);
            state2 ^= LoadBigWord(&m_reductionTable[highNibble][2]);
            state3 ^= LoadBigWord(&m_reductionTable[highNibble][3]);
        }
        else
        {
            const uint32_t old3 = state3;
            const uint32_t old0 = state0;
            const uint32_t rotated2 = (state2 >> 4) | (state1 << 28);
            const uint32_t rotated3 = (state2 << 28) | (old3 >> 4);
            state2 = rotated2;
            const uint32_t shifted0 = old0 >> 4;
            state3 = rotated3;
            state0 = shifted0;
            const uint32_t shifted1 = (old0 << 28) | (state1 >> 4);
            state1 = shifted1;
            state0 = shifted0 ^ reduction[old3 & 0x0fu];
            state0 ^= low0;
            state1 = shifted1 ^ low1;

            value0 = state0;
            value1 = state1;
            value2 = state2 ^ low2;
            value3 = state3 ^ low3;
            const uint32_t next1 = (value0 << 28) | (value1 >> 4);
            const uint32_t next0 = value0 >> 4;
            const uint32_t next2 = (value2 >> 4) | (value1 << 28);
            const uint32_t next3 = (value2 << 28) | (value3 >> 4);
            state0 = next0;
            state1 = next1;
            state2 = next2;
            state3 = next3;
            state0 ^= reduction[value3 & 0x0fu];
            state0 ^= LoadBigWord(&m_reductionTable[highNibble][0]);
            state1 ^= LoadBigWord(&m_reductionTable[highNibble][1]);
            state2 ^= LoadBigWord(&m_reductionTable[highNibble][2]);
            state3 ^= LoadBigWord(&m_reductionTable[highNibble][3]);
        }

        StoreBigWord(&m_hashKeyWords[0], state0);
        StoreBigWord(&m_hashKeyWords[1], state1);
        StoreBigWord(&m_hashKeyWords[2], state2);
        StoreBigWord(&m_hashKeyWords[3], state3);
        --reverseIndex;
        --remaining;
    }

    StoreBigWord(block->bytes + 0, state0);
    StoreBigWord(block->bytes + 4, state1);
    StoreBigWord(block->bytes + 8, state2);
    StoreBigWord(block->bytes + 12, state3);
}

void GcmModeImpl<GcmModeGenericHelper>::Reset(const void* initializationVector,
                                               unsigned int size)
{
    memset(&m_hash, 0, sizeof(m_hash));
    m_aadSizeLow = 0;
    m_aadSizeHigh = 0;
    m_textSizeLow = 0;
    m_textSizeHigh = 0;
    m_keystreamOffset = 0;
    m_partialSize = 0;
    m_blockCounter = 0;

    if (size == 12)
    {
        memcpy(m_counter.bytes, initializationVector, size);
        m_counter.bytes[12] = 0;
        m_counter.bytes[13] = 0;
        m_counter.bytes[14] = 0;
        m_counter.bytes[15] = 1;
        m_blockCounter = 1;
    }
    else
    {
        memset(&m_counter, 0, sizeof(m_counter));
        const uint8_t* bytes = static_cast<const uint8_t*>(initializationVector);
        const unsigned int completeBlocks = size / sizeof(m_hash.bytes);
        const unsigned int remainder = size % sizeof(m_hash.bytes);
        for (unsigned int block = 0; block < completeBlocks; ++block)
        {
            for (unsigned int index = 0; index < sizeof(m_hash.bytes); ++index)
                m_counter.bytes[index] ^= bytes[block * sizeof(m_counter.bytes) + index];
            GfMultH(&m_counter);
        }
        for (unsigned int index = 0; index < remainder; ++index)
            m_counter.bytes[index] ^= bytes[completeBlocks * sizeof(m_counter.bytes) + index];
        if (remainder != 0)
            GfMultH(&m_counter);
        const uint32_t bitLength = size << 3;
        m_counter.bytes[12] ^= static_cast<uint8_t>(bitLength >> 24);
        m_counter.bytes[13] ^= static_cast<uint8_t>(bitLength >> 16);
        m_counter.bytes[14] ^= static_cast<uint8_t>(bitLength >> 8);
        m_counter.bytes[15] ^= static_cast<uint8_t>(bitLength);
        GfMultH(&m_counter);
    }

    if (m_encryptBlock != 0)
        m_encryptBlock(&m_mac, &m_counter, m_cipher);
    m_state = 2;
}

unsigned int GcmModeImpl<GcmModeGenericHelper>::EncryptUpdate(
    void* output, unsigned int outputSize, const void* input, unsigned int size)
{
    (void)outputSize;
    if (m_partialSize != 0)
    {
        GfMultH(&m_hash);
        m_partialSize = 0;
    }

    uint8_t* destination = static_cast<uint8_t*>(output);
    const uint8_t* source = static_cast<const uint8_t*>(input);
    m_state = 3;
    for (unsigned int index = 0; index < size; ++index)
    {
        if (m_keystreamOffset == 0)
        {
            ++m_blockCounter;
            for (unsigned int counter = sizeof(m_counter.bytes); counter != 12; --counter)
                if (++m_counter.bytes[counter - 1] != 0)
                    break;
            if (m_encryptBlock != 0)
                m_encryptBlock(&m_keystream, &m_counter, m_cipher);
        }
        const unsigned int streamIndex = m_keystreamOffset;
        m_keystreamOffset = (m_keystreamOffset + 1u) & 15u;
        destination[index] = static_cast<uint8_t>(source[index] ^ m_keystream.bytes[streamIndex]);
        m_hash.bytes[streamIndex & 15u] ^= destination[index];
        if ((streamIndex & 15u) == 15u)
            GfMultH(&m_hash);
    }

    const uint32_t oldTextSize = m_textSizeLow;
    m_textSizeLow += size;
    m_textSizeHigh += (m_textSizeLow < oldTextSize) ? 1u : 0u;
    return size;
}

unsigned int GcmModeImpl<GcmModeGenericHelper>::DecryptUpdate(
    void* output, unsigned int outputSize, const void* input, unsigned int size)
{
    (void)outputSize;
    if (m_partialSize != 0)
    {
        GfMultH(&m_hash);
        m_partialSize = 0;
    }

    uint8_t* destination = static_cast<uint8_t*>(output);
    const uint8_t* source = static_cast<const uint8_t*>(input);
    m_state = 4;
    for (unsigned int index = 0; index < size; ++index)
    {
        if (m_keystreamOffset == 0)
        {
            ++m_blockCounter;
            for (unsigned int counter = sizeof(m_counter.bytes); counter != 12; --counter)
                if (++m_counter.bytes[counter - 1] != 0)
                    break;
            if (m_encryptBlock != 0)
                m_encryptBlock(&m_keystream, &m_counter, m_cipher);
        }
        const unsigned int streamIndex = m_keystreamOffset;
        m_keystreamOffset = (m_keystreamOffset + 1u) & 15u;
        destination[index] = static_cast<uint8_t>(source[index] ^ m_keystream.bytes[streamIndex]);
        m_hash.bytes[streamIndex & 15u] ^= source[index];
        if ((streamIndex & 15u) == 15u)
            GfMultH(&m_hash);
    }

    const uint32_t oldTextSize = m_textSizeLow;
    m_textSizeLow += size;
    m_textSizeHigh += (m_textSizeLow < oldTextSize) ? 1u : 0u;
    return size;
}

void GcmModeImpl<GcmModeGenericHelper>::GenerateMac()
{
    if (m_keystreamOffset != 0 || m_partialSize != 0)
        GfMultH(&m_hash);

    const uint32_t aadHigh = (m_aadSizeHigh << 3) | (m_aadSizeLow >> 29);
    const uint32_t aadLow = m_aadSizeLow << 3;
    const uint32_t textHigh = (m_textSizeHigh << 3) | (m_textSizeLow >> 29);
    const uint32_t textLow = m_textSizeLow << 3;
    StoreBigWord(&m_hashKeyWords[1], aadLow);
    StoreBigWord(&m_hashKeyWords[0], aadHigh);
    StoreBigWord(&m_hashKeyWords[2], textHigh);
    StoreBigWord(&m_hashKeyWords[3], textLow);
    uint32_t hashWords[4] = {
        LoadBigWord(m_hash.bytes + 0), LoadBigWord(m_hash.bytes + 4),
        LoadBigWord(m_hash.bytes + 8), LoadBigWord(m_hash.bytes + 12)
    };
    const uint32_t macWords[4] = {
        LoadBigWord(m_mac.bytes + 0), LoadBigWord(m_mac.bytes + 4),
        LoadBigWord(m_mac.bytes + 8), LoadBigWord(m_mac.bytes + 12)
    };
    hashWords[0] ^= aadHigh;
    hashWords[1] ^= aadLow;
    hashWords[2] ^= textHigh;
    hashWords[3] ^= textLow;
    for (unsigned int word = 0; word < 4; ++word)
        StoreBigWord(m_hash.bytes + word * 4, hashWords[word]);
    GfMultH(&m_hash);
    for (unsigned int word = 0; word < 4; ++word)
    {
        hashWords[word] = LoadBigWord(m_hash.bytes + word * 4) ^ macWords[word];
        StoreBigWord(m_hash.bytes + word * 4, hashWords[word]);
    }
}

void GcmModeImpl<GcmModeGenericHelper>::GetMac(void* output, unsigned int size)
{
    if (m_state != 5)
    {
        GenerateMac();
        m_state = 5;
    }
    (void)size;
    memcpy(output, m_hash.bytes, sizeof(m_hash.bytes));
}

void GcmModeImpl<GcmModeGenericHelper>::InitializeHashKey()
{
    memset(m_hashKeyWords, 0, sizeof(m_hashKeyWords));
    if (m_encryptBlock != 0)
        m_encryptBlock(m_hashKeyWords, m_hashKeyWords, m_cipher);

    memset(m_reductionTable, 0, sizeof(m_reductionTable));
    uint32_t words[4];
    for (unsigned int word = 0; word < 4; ++word)
        words[word] = LoadBigWord(&m_hashKeyWords[word]);
    for (unsigned int word = 0; word < 4; ++word)
        StoreBigWord(&m_reductionTable[8][word], words[word]);
    for (unsigned int index = 4; index != 0; index >>= 1)
    {
        const bool reduction = (words[3] & 1u) != 0;
        words[3] = (words[3] >> 1) | (words[2] << 31);
        words[2] = (words[2] >> 1) | (words[1] << 31);
        words[1] = (words[1] >> 1) | (words[0] << 31);
        words[0] >>= 1;
        if (reduction)
            words[0] ^= 0xe1000000u;
        for (unsigned int word = 0; word < 4; ++word)
            StoreBigWord(&m_reductionTable[index][word], words[word]);
    }
    for (unsigned int base = 2; base < 9; base <<= 1)
    {
        for (unsigned int offset = 1; offset < base; ++offset)
        {
            for (unsigned int word = 0; word < 4; ++word)
            {
                const uint32_t value =
                    LoadBigWord(&m_reductionTable[base][word]) ^
                    LoadBigWord(&m_reductionTable[offset][word]);
                StoreBigWord(&m_reductionTable[base + offset][word], value);
            }
        }
    }
}

}}}
