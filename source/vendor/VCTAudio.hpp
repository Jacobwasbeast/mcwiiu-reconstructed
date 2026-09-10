#ifndef SOURCE_VENDOR_VCT_AUDIO_HPP
#define SOURCE_VENDOR_VCT_AUDIO_HPP

enum VCTCodec
{
    VCT_CODEC_0 = 0,
    VCT_CODEC_1,
    VCT_CODEC_2,
    VCT_CODEC_3,
    VCT_CODEC_4,
    VCT_CODEC_5,
    VCT_CODEC_6
};

struct _udp_packet;

namespace nn
{
namespace pia
{
namespace chat
{
class AudioStream;
}
}
}

struct VCTSession
{
    unsigned int m_Mode;
    unsigned short m_Unknown04;
    unsigned short m_Cid;
    unsigned short m_OwnerCid;
    unsigned short m_ConferenceCids[16];
    unsigned short m_Unknown2A;
    unsigned int m_State;
    unsigned int m_Unknown30;
    unsigned int m_Unknown34;
    VCTSession* m_Next;
};

struct VCTAudioInfo
{
    unsigned int m_Unknown00;
    unsigned int m_Unknown04;
    unsigned int m_LastSequence;
    unsigned int m_Unknown0C;
    long long m_JitterOffset;
    long long m_JitterAverage;
    long long m_Skew;
    unsigned int m_Latency;
    unsigned int m_QueuedPacketCount;
    unsigned int m_DropCount;
    unsigned int m_ContinuousDropCount;
    unsigned int m_OutOfOrderCount;
    unsigned int m_DuplicateCount;
    unsigned int m_TooEarlyCount;
    unsigned int m_LateCount;
    unsigned int m_ExpiredCount;
    unsigned int m_Unknown4C;
    unsigned int m_Unknown50;
    unsigned int m_FadeRecoverCount;
    unsigned int m_SpareRecoverCount;
    unsigned int m_Unknown5C;
};

bool VCT_IsInitialized();

unsigned int VCTi_GetAudioFrameLength();
unsigned int VCTi_GetAudioDataSize();
long long VCTi_GetAudioFrameTicks();
unsigned int VCT_GetAudioFrameLength();
unsigned int VCT_GetAudioDataSize();

void VCT_ResetVAD();
unsigned int VCT_GetVADReleaseTime();
unsigned char VCT_GetVADActivity();
unsigned int VCTi_GetVADStatus(unsigned int energy, unsigned int threshold, unsigned char* pStatus);
void vct_init_vad();
unsigned short VCT_GetVADActiveGain();
unsigned short VCT_GetVADInactiveGain();
unsigned int VCT_GetVADClampGain();
unsigned int VCT_IsEnableVADInvalidation();

bool vct_is_adpcm(VCTCodec codec);
bool vct_is_opus(VCTCodec codec);
void init_adpcm_state();
int vct_get_packet_size(unsigned char ucCodec);

unsigned int HPFFilter2(const void* pAudio, unsigned int uiSize);
unsigned int VCT_GetAudioScale(const void* pAudio, unsigned int uiSize);

int VCT_SendAudioEx(const void* pAudio, unsigned int uiAudioScale);
int VCT_ReceiveAudio(void* pData, unsigned int uiSize, unsigned short* pCids);
int VCTi_ReceiveAudioData(void* pData, unsigned int uiSize, unsigned int uiStreamIndex,
                          unsigned short* pCid);
void vct_mix_audio(const short* pLeft, const short* pRight, short* pOut, unsigned int uiSize);

nn::pia::chat::AudioStream* vct_get_audio_stream(unsigned int uiStreamIndex);
nn::pia::chat::AudioStream* VCTi_GetStreamFromCid(unsigned short usCid);
unsigned int VCT_GetDropCount(unsigned short usCid);
unsigned int VCT_GetDropFrequency(unsigned short usCid);
void VCT_GetAudioInfoFromCid(unsigned short usCid, VCTAudioInfo* pAudioInfo);

VCTSession* vct_find_session(unsigned short usCid);
VCTSession* VCTi_CreateSession(unsigned short usCid);
int VCT_DeleteSession(VCTSession* pSession);

int VCT_StartStreaming(const VCTSession* pSession);
void VCT_ClearStreaming(const VCTSession* pSession);
void VCT_StopStreaming(const VCTSession* pSession);

int VCT_AddConferenceClient(unsigned short usCid);
int VCT_RemoveConferenceClient(unsigned short usCid);
int VCT_OnConferenceClient(unsigned short usCid);

void VCT_ResetVAD();

#endif
