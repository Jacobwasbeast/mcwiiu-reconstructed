#include "nn/pia/chat/Adpcm.hpp"

namespace
{

const signed char g_adpcm_delta_3[8] = {-1, -1, 1, 2, -1, -1, 1, 2};
const signed char g_adpcm_delta_2[4] = {-1, 1, -1, 1};
const signed char g_adpcm_delta_4[16] = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
const short g_adpcm_steps[] = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,    19,    21,   23,   25,   28,    31,
    34,    37,    41,    45,    50,    55,    60,    66,    72,    79,    86,    95,   104,  114,  125,   137,
    150,   164,   180,   197,   216,   237,   259,   284,   311,   341,   374,   410,  449,  492,  539,   591,
    648,   711,   779,   854,   936,   1026,  1125,  1232,  1352,  1482,  1625,  1782, 1955, 2142, 2349,  2571,
    2822,  3095,  3395,  3721,  4085,  4481,  4912,  5382,  5900,  6470,  7099,  7790, 8547, 9384, 10300, 11300,
    12400, 13600, 14900, 16400, 18000, 19700, 21600, 23700, 26000, 28500, 31300, 32767};

typedef nn::pia::chat::Adpcm::State AdpcmState;

inline int adpcm_clip(int value)
{
    if (value < -32768)
        return -32768;
    if (value > 32767)
        return 32767;
    return value;
}

#define ADPCM_DECODE_2(CODE, STATE, OUT) \
    { \
        int idx = (STATE)->index; \
        int sample = (STATE)->sample; \
        int step = g_adpcm_steps[idx]; \
        int diff = ((CODE) & 1) ? step : 0; \
        if ((CODE) & 2) \
            sample -= diff; \
        else \
            sample += diff; \
        sample = adpcm_clip(sample); \
        idx += g_adpcm_delta_2[CODE]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
        (OUT) = sample; \
    }

#define ADPCM_DECODE_3(CODE, STATE, OUT) \
    { \
        int idx = (STATE)->index; \
        int sample = (STATE)->sample; \
        int step = g_adpcm_steps[idx]; \
        int diff = step >> 2; \
        if ((CODE) & 2) \
            diff += step; \
        if ((CODE) & 1) \
            diff += step >> 1; \
        if ((CODE) & 4) \
            sample -= diff; \
        else \
            sample += diff; \
        sample = adpcm_clip(sample); \
        idx += g_adpcm_delta_3[(CODE) & 7]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
        (OUT) = sample; \
    }

#define ADPCM_DECODE_4(CODE, STATE, OUT) \
    { \
        int idx = (STATE)->index; \
        int sample = (STATE)->sample; \
        int step = g_adpcm_steps[idx]; \
        int diff = step >> 3; \
        if ((CODE) & 4) \
            diff += step; \
        if ((CODE) & 2) \
            diff += step >> 1; \
        if ((CODE) & 1) \
            diff += step >> 2; \
        if ((CODE) & 8) \
            sample -= diff; \
        else \
            sample += diff; \
        sample = adpcm_clip(sample); \
        idx += g_adpcm_delta_4[CODE]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
        (OUT) = sample; \
    }

#define ADPCM_ENCODE_2(TARGET, STATE, CODE) \
    { \
        int error = (TARGET) - (STATE)->sample; \
        (CODE) = 0; \
        if (error < 0) \
        { \
            error = -error; \
            (CODE) = 2; \
        } \
        int quant = g_adpcm_steps[(STATE)->index]; \
        if (error >= quant) \
            (CODE) |= 1; \
        int diff = ((CODE) & 1) ? quant : 0; \
        if ((CODE) & 2) \
            diff = -diff; \
        int sample = (STATE)->sample + diff; \
        sample = adpcm_clip(sample); \
        int idx = (STATE)->index + g_adpcm_delta_2[CODE]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
    }

#define ADPCM_ENCODE_3(TARGET, STATE, CODE) \
    { \
        int error = (TARGET) - (STATE)->sample; \
        (CODE) = 0; \
        if (error < 0) \
        { \
            error = -error; \
            (CODE) = 4; \
        } \
        int quant = g_adpcm_steps[(STATE)->index]; \
        if (error >= quant) \
        { \
            error -= quant; \
            (CODE) |= 2; \
        } \
        if (error >= (quant >> 1)) \
            (CODE) |= 1; \
        int diff = quant >> 2; \
        if ((CODE) & 2) \
            diff += quant; \
        if ((CODE) & 1) \
            diff += quant >> 1; \
        if ((CODE) & 4) \
            diff = -diff; \
        int sample = (STATE)->sample + diff; \
        sample = adpcm_clip(sample); \
        int idx = (STATE)->index + g_adpcm_delta_3[(CODE) & 7]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
    }

#define ADPCM_ENCODE_4(TARGET, STATE, CODE) \
    { \
        int error = (TARGET) - (STATE)->sample; \
        (CODE) = 0; \
        if (error < 0) \
        { \
            error = -error; \
            (CODE) = 8; \
        } \
        int quant = g_adpcm_steps[(STATE)->index]; \
        if (error >= quant) \
        { \
            error -= quant; \
            (CODE) |= 4; \
        } \
        int half = quant >> 1; \
        if (error >= half) \
        { \
            error -= half; \
            (CODE) |= 2; \
        } \
        if (error >= (quant >> 2)) \
            (CODE) |= 1; \
        int diff = quant >> 3; \
        if ((CODE) & 4) \
            diff += quant; \
        if ((CODE) & 2) \
            diff += half; \
        if ((CODE) & 1) \
            diff += quant >> 2; \
        if ((CODE) & 8) \
            diff = -diff; \
        int sample = (STATE)->sample + diff; \
        sample = adpcm_clip(sample); \
        int idx = (STATE)->index + g_adpcm_delta_4[CODE]; \
        if (idx < 0) \
            idx = 0; \
        else if (idx > 88) \
            idx = 88; \
        (STATE)->sample = sample; \
        (STATE)->index = idx; \
    }

void vct_decode_adpcm_16(const unsigned char* input, short* output, unsigned int count, AdpcmState* state)
{
    for (; count != 0; --count)
    {
        ADPCM_DECODE_2(*input & 3, state, *output++)
        ADPCM_DECODE_2((*input >> 2) & 3, state, *output++)
        ADPCM_DECODE_2((*input >> 4) & 3, state, *output++)
        ADPCM_DECODE_2((*input >> 6) & 3, state, *output++)
        input++;
    }
}

void vct_decode_adpcm_24(const unsigned char* input, short* output, unsigned int count, AdpcmState* state)
{
    for (count /= 3; count != 0; --count)
    {
        ADPCM_DECODE_3(input[0] >> 5, state, *output++)
        ADPCM_DECODE_3((input[0] >> 2) & 7, state, *output++)
        ADPCM_DECODE_3(((input[0] & 3) << 1) | (input[1] >> 7), state, *output++)
        ADPCM_DECODE_3((input[1] >> 4) & 7, state, *output++)
        ADPCM_DECODE_3((input[1] >> 1) & 7, state, *output++)
        ADPCM_DECODE_3(((input[1] & 1) << 2) | (input[2] >> 6), state, *output++)
        ADPCM_DECODE_3((input[2] >> 3) & 7, state, *output++)
        ADPCM_DECODE_3(input[2] & 7, state, *output++)
        input += 3;
    }
}

void vct_decode_adpcm_32(const unsigned char* input, short* output, unsigned int count, AdpcmState* state)
{
    for (; count != 0; --count)
    {
        ADPCM_DECODE_4(*input & 15, state, *output++)
        ADPCM_DECODE_4((*input >> 4) & 15, state, *output++)
        input++;
    }
}

void vct_encode_adpcm_16(unsigned char* output, const short* input, unsigned int count, AdpcmState* state)
{
    for (count >>= 2; count != 0; --count)
    {
        unsigned int c0, c1, c2, c3;
        unsigned char packed;
        ADPCM_ENCODE_2(*input++, state, c0)
        packed = c0;
        ADPCM_ENCODE_2(*input++, state, c1)
        packed = packed | (c1 << 2);
        ADPCM_ENCODE_2(*input++, state, c2)
        packed = packed | (c2 << 4);
        ADPCM_ENCODE_2(*input++, state, c3)
        *output++ = packed | (c3 << 6);
    }
}

void vct_encode_adpcm_24(unsigned char* output, const short* input, unsigned int count, AdpcmState* state)
{
    for (count >>= 3; count != 0; --count)
    {
        unsigned int c0, c1, c2, c3, c4, c5, c6, c7;
        ADPCM_ENCODE_3(*input++, state, c0)
        ADPCM_ENCODE_3(*input++, state, c1)
        ADPCM_ENCODE_3(*input++, state, c2)
        ADPCM_ENCODE_3(*input++, state, c3)
        ADPCM_ENCODE_3(*input++, state, c4)
        ADPCM_ENCODE_3(*input++, state, c5)
        ADPCM_ENCODE_3(*input++, state, c6)
        ADPCM_ENCODE_3(*input++, state, c7)
        output[0] = static_cast<unsigned char>((c0 << 5) | (c1 << 2) | (c2 >> 1));
        output[1] = static_cast<unsigned char>((c2 << 7) | (c3 << 4) | (c4 << 1) | (c5 >> 2));
        output[2] = static_cast<unsigned char>((c5 << 6) | (c6 << 3) | c7);
        output += 3;
    }
}

void vct_encode_adpcm_32(unsigned char* output, const short* input, unsigned int count, AdpcmState* state)
{
    for (count >>= 1; count != 0; --count)
    {
        unsigned int c0, c1;
        ADPCM_ENCODE_4(*input++, state, c0)
        ADPCM_ENCODE_4(*input++, state, c1)
        *output++ = static_cast<unsigned char>(c0 | (c1 << 4));
    }
}

}

void nn::pia::chat::Adpcm::Decode(const unsigned char* pData, short* pOut, unsigned int uiSize, VCTCodec codec)
{
    const State* header = reinterpret_cast<const State*>(pData);
    State state;
    state.sample = header->sample;
    state.index = header->index;
    pData += sizeof(State);
    uiSize -= sizeof(State);

    switch (codec)
    {
    case VCT_CODEC_2:
        vct_decode_adpcm_16(pData, pOut, uiSize, &state);
        break;
    case VCT_CODEC_3:
        vct_decode_adpcm_24(pData, pOut, uiSize, &state);
        break;
    case VCT_CODEC_4:
        vct_decode_adpcm_32(pData, pOut, uiSize, &state);
        break;
    default:
        break;
    }
}

void nn::pia::chat::Adpcm::Encode(unsigned char* pOut, const short* pAudio, unsigned int uiSize, State* pState,
                                  VCTCodec codec)
{
    State* header = reinterpret_cast<State*>(pOut);
    header->sample = pState->sample;
    header->index = pState->index;
    header->reserved = 0;
    pOut += sizeof(State);

    switch (codec)
    {
    case VCT_CODEC_2:
        vct_encode_adpcm_16(pOut, pAudio, uiSize, pState);
        break;
    case VCT_CODEC_3:
        vct_encode_adpcm_24(pOut, pAudio, uiSize, pState);
        break;
    case VCT_CODEC_4:
        vct_encode_adpcm_32(pOut, pAudio, uiSize, pState);
        break;
    default:
        break;
    }
}
