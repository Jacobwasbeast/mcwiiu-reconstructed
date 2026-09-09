#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_CODECFACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_CODECFACADE_HPP

#include "vendor/VCTAudio.hpp"

namespace nn { namespace pia { namespace chat {

class CodecFacade
{
public:
    CodecFacade();
    virtual ~CodecFacade();
    virtual void Trace(unsigned long long value) const;
    void Initialize();
    void Finalize();
    void Decode(const unsigned char* pData, short* pOut, unsigned int uiSize, VCTCodec codec);
    void Encode(unsigned char* pOut, const short* pAudio, unsigned int uiSize, void* pState,
                VCTCodec codec);
    static void CreateInstance();
    static void DestroyInstance();

private:
    unsigned int m_state;
};

extern CodecFacade* g_codecFacade;
extern void* g_codecStorage;

}}}

void vct_encode_g711_ulaw(unsigned char* pOut, const short* pAudio, unsigned int uiCount);
void vct_decode_g711_ulaw(const unsigned char* pData, short* pOut, unsigned int uiCount);
void vct_encode_8bit_raw(unsigned char* pOut, const short* pAudio, unsigned int uiCount);
void vct_decode_8bit_raw(const unsigned char* pData, short* pOut, unsigned int uiCount);

#endif
