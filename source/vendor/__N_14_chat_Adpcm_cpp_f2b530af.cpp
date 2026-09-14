#include "vendor/__N_14_chat_Adpcm_cpp_f2b530af.hpp"

static const short g_adpcm_steps[] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,72,79,86,95,104,114,125,137,150,164,180,197,216,237,259,284,311,341,374,410,449,492,539,591,648,711,779,854,936,1026,1125,1232,1352,1482,1625,1782,1955,2142,2349,2571,2822,3095,3395,3721,4085,4481,4912,5382,5900,6470,7099,7790,8547,9384,10300,11300,12400,13600,14900,16400,18000,19700,21600,23700,26000,28500,31300,32767
};
static const signed char g_adpcm_delta_2[4] = {-1,1,-1,1};
static const signed char g_adpcm_delta_3[8] = {-1,-1,-1,-1,2,4,6,8};
static const signed char g_adpcm_delta_4[16] = {-1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8};

static short adpcm_clip(int value)
{
    if (value < -32768) return -32768;
    if (value > 32767) return 32767;
    return (short)value;
}

static short adpcm_decode(unsigned int code, unsigned int bits, nn::pia::chat::Adpcm::State* state)
{
    int step = g_adpcm_steps[state->index];
    int magnitude;
    int newIndex;
    if (bits == 2)
        magnitude = (code & 1) ? step : 0;
    else if (bits == 3)
        magnitude = (step >> 2) + ((code & 2) ? step : 0) + ((code & 1) ? (step >> 1) : 0);
    else
        magnitude = (step >> 3) + ((code & 4) ? step : 0) + ((code & 2) ? (step >> 1) : 0) + ((code & 1) ? (step >> 2) : 0);
    if (code & (1u << (bits - 1))) magnitude = -magnitude;
    state->sample = adpcm_clip((int)state->sample + magnitude);
    if (bits == 2) newIndex = (int)state->index + g_adpcm_delta_2[code & 3];
    else if (bits == 3) newIndex = (int)state->index + g_adpcm_delta_3[code & 7];
    else newIndex = (int)state->index + g_adpcm_delta_4[code & 15];
    if (newIndex < 0) newIndex = 0;
    else if (newIndex > 88) newIndex = 88;
    state->index = newIndex;
    return state->sample;
}

static unsigned int adpcm_encode(short target, unsigned int bits, nn::pia::chat::Adpcm::State* state)
{
    int difference = (int)target - state->sample;
    unsigned int code = 0;
    int magnitude = difference;
    int step = g_adpcm_steps[state->index];
    if (magnitude < 0) { magnitude = -magnitude; code = 1u << (bits - 1); }
    if (bits == 2) { if (magnitude >= step) code |= 1; }
    else if (bits == 3) { if (magnitude >= step) { code |= 2; magnitude -= step; } if (magnitude >= (step >> 1)) code |= 1; }
    else { if (magnitude >= step) { code |= 4; magnitude -= step; } if (magnitude >= (step >> 1)) { code |= 2; magnitude -= (step >> 1); } if (magnitude >= (step >> 2)) code |= 1; }
    adpcm_decode(code, bits, state);
    return code;
}

static void adpcm_decode_codes(const unsigned char* input, short* output, unsigned int count, unsigned int bits, nn::pia::chat::Adpcm::State* state)
{
    unsigned int i;
    if (bits == 2) for (i = 0; i < count; ++i) { unsigned char b = input[i >> 2]; output[i] = adpcm_decode((b >> ((i & 3) * 2)) & 3, bits, state); }
    else if (bits == 4) for (i = 0; i < count; ++i) { unsigned char b = input[i >> 1]; output[i] = adpcm_decode((b >> ((i & 1) * 4)) & 15, bits, state); }
}

void __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_16(const unsigned char* input, short* output, unsigned int count, nn::pia::chat::Adpcm::State* state) { adpcm_decode_codes(input, output, count * 4, 2, state); }
void __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_32(const unsigned char* input, short* output, unsigned int count, nn::pia::chat::Adpcm::State* state) { adpcm_decode_codes(input, output, count * 2, 4, state); }

void __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_24(const unsigned char* input, short* output, unsigned int count, nn::pia::chat::Adpcm::State* state)
{
    unsigned int i, j; unsigned char b0, b1, b2; unsigned int code[8];
    for (i = 0; i < count / 3; ++i) { b0=input[i*3]; b1=input[i*3+1]; b2=input[i*3+2]; code[0]=b0>>5; code[1]=(b0>>2)&7; code[2]=((b0&3)<<1)|(b1>>7); code[3]=(b1>>4)&7; code[4]=(b1>>1)&7; code[5]=((b1&1)<<2)|(b2>>6); code[6]=(b2>>3)&7; code[7]=b2&7; for(j=0;j<8;++j) output[i*8+j]=adpcm_decode(code[j],3,state); }
}

void nn::pia::chat::Adpcm::Decode(const unsigned char* pData, short* pOut, unsigned int uiSize, VCTCodec codec)
{
    const unsigned int headerSize = sizeof(State);
    const State* header = reinterpret_cast<const State*>(pData);
    State state;
    state.sample = header->sample;
    state.index = header->index;

    switch (codec)
    {
    case VCT_CODEC_2:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_16(pData + headerSize, pOut, uiSize - headerSize, &state);
        break;
    case VCT_CODEC_3:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_24(pData + headerSize, pOut, uiSize - headerSize, &state);
        break;
    case VCT_CODEC_4:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_decode_adpcm_32(pData + headerSize, pOut, uiSize - headerSize, &state);
        break;
    default:
        break;
    }
}

static void adpcm_encode_codes(unsigned char* output, const short* input, unsigned int count, unsigned int bits, nn::pia::chat::Adpcm::State* state)
{
    unsigned int i; for (i=0;i<count/(8/bits);++i) { unsigned int j; unsigned char b=0; for(j=0;j<8/bits;++j) b |= (unsigned char)(adpcm_encode(input[i*(8/bits)+j],bits,state) << (j*bits)); output[i]=b; }
}
void __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_16(unsigned char* output, const short* input, unsigned int count, nn::pia::chat::Adpcm::State* state) { adpcm_encode_codes(output,input,count,2,state); }
void __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_32(unsigned char* output, const short* input, unsigned int count, nn::pia::chat::Adpcm::State* state) { adpcm_encode_codes(output,input,count,4,state); }
void __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_24(unsigned char* output, const short* input, unsigned int count, nn::pia::chat::Adpcm::State* state)
{
    unsigned int i,j; for(i=0;i<count/8;++i) { unsigned int code[8]; unsigned char b0=0,b1=0,b2=0; for(j=0;j<8;++j) code[j]=adpcm_encode(input[i*8+j],3,state); b0=(unsigned char)((code[0]<<5)|(code[1]<<2)|(code[2]>>1)); b1=(unsigned char)((code[2]<<7)|(code[3]<<4)|(code[4]<<1)|(code[5]>>2)); b2=(unsigned char)((code[5]<<6)|(code[6]<<3)|code[7]); output[i*3]=b0; output[i*3+1]=b1; output[i*3+2]=b2; }
}

void nn::pia::chat::Adpcm::Encode(unsigned char* pOut, const short* pAudio, unsigned int uiSize, State* pState, VCTCodec codec)
{
    const unsigned int headerSize = sizeof(State);
    State* header = reinterpret_cast<State*>(pOut);
    header->sample = pState->sample;
    header->index = pState->index;
    header->reserved = 0;

    switch (codec)
    {
    case VCT_CODEC_2:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_16(pOut + headerSize, pAudio, uiSize, pState);
        break;
    case VCT_CODEC_3:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_24(pOut + headerSize, pAudio, uiSize, pState);
        break;
    case VCT_CODEC_4:
        __N_14_chat_Adpcm_cpp_f2b530af::vct_encode_adpcm_32(pOut + headerSize, pAudio, uiSize, pState);
        break;
    default:
        break;
    }
}
