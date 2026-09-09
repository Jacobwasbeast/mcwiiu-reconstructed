#ifndef SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP
#define SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP

#include <stdint.h>
#include "nn/pia.h"
#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia {
typedef int Result;
namespace transport {
StationIndex Conv2StationIndex(unsigned long long stationId);
unsigned char IsInitialized();
}
namespace chat {

struct VCTSession;
class HandsFree;

enum Codec { CODEC_0 = 0, CODEC_1, CODEC_2, CODEC_3, CODEC_4, CODEC_5, CODEC_6 };
enum EcMode { EC_MODE_0 = 0, EC_MODE_1, EC_MODE_2, EC_MODE_3 };

struct AudioInfo {
    unsigned int m_Unknown0;
    unsigned int m_Unknown1;
    unsigned int m_Unknown2;
    unsigned int m_Unknown3;
    unsigned int m_Sequence;
    unsigned char m_UnknownTail[76];
};

class VoiceProtocol : public nn::pia::transport::Protocol
{
public:
    struct Setting {
        unsigned int bufferSize;
        unsigned int maxPlayers;
        unsigned int dataSize;
        unsigned char codec;
        unsigned char reserved[3];
        unsigned int frameLength;
        unsigned short sampleRate;
        unsigned short settingTailPadding;
    };

    VoiceProtocol();
    ~VoiceProtocol() override;
    Result Initialize(const Setting& setting);
    void ClearStreaming();
    Result AddConferenceClient(StationIndex stationIndex);
    Result AddConferenceClient(unsigned long long stationId);
    Result RemoveConferenceClient(StationIndex stationIndex);
    Result RemoveConferenceClient(unsigned long long stationId);
    Result Send(const short* pData, unsigned int uiSamples,
                const short* pLeft, const short* pRight);
    Result Receive(void* pData, unsigned int uiSize, StationIndex* pStationIndex);
    void Finalize();
    Result GetAudioInfo(StationIndex stationIndex, AudioInfo* audioInfo) const;
    Result GetAudioInfo(unsigned long long stationId, AudioInfo* audioInfo) const;
    unsigned char GetVadActivity() const;
    unsigned int GetAudioDataSize() const;
    bool IsInCommunication() const;
    unsigned int GetAudioFrameLength() const;
    bool IsClientInConference(StationIndex stationIndex) const;
    bool IsClientInConference(unsigned long long stationId) const;

private:
    int calcAudioBufferSize(unsigned short sampleRate) const;
    Result allocProc(unsigned short sampleRate);
    Result setupProtocols(unsigned char maxPlayers, unsigned int dataSize);
    void freeProc();
    void destroyProtocols();
    Result receiveCore(void* pData, unsigned int uiSize,
                       unsigned short* pStationIndex, void* pCidBuffer);

    unsigned int m_secondaryState;
    unsigned int m_secondaryStatus;
    VCTSession* m_vctSession;
    void* m_auxiliarySession;
    StationIndex m_stationIndex;
    unsigned int m_codec;
    unsigned int m_bufferSize;
    unsigned int m_maxPlayers;
    unsigned int m_dataSize;
    unsigned int m_sampleRate;
    nn::pia::common::CriticalSection m_criticalSection;
    unsigned int m_streamingState;
    unsigned int m_monitoringBytesHigh;
    unsigned int m_monitoringBytesLow;
    unsigned int m_monitoringSpeechStart;
    unsigned int m_monitoringSpeechEnd;
    unsigned int m_monitoringTicksHigh;
    unsigned int m_monitoringTicksLow;
    HandsFree* m_handsFree;
    void* m_pead;
};

unsigned char IsInitialized();
unsigned short ConvertID_Pia2VCT(StationIndex stationIndex);
}
}}

extern unsigned int g_voiceAudioFrameLength;
extern unsigned int g_voiceAudioDataSize;
extern unsigned char g_voiceVadActivity;
extern nn::pia::chat::VoiceProtocol* g_voiceProtocolInstance;
extern void VCT_ClearStreaming(nn::pia::chat::VCTSession* session);


enum VCTCodec { VCT_CODEC_0 = 0, VCT_CODEC_1, VCT_CODEC_2, VCT_CODEC_3, VCT_CODEC_4, VCT_CODEC_5, VCT_CODEC_6 };
enum TMC_HF_EC_MODE { TMC_HF_EC_MODE_0 = 0, TMC_HF_EC_MODE_1, TMC_HF_EC_MODE_2 };

class __N_22_chat_VoiceProtocol_cpp_bf22a0fe
{
public:
    static void lock_();
    static void unlock_();
    static bool isOpus(nn::pia::chat::Codec codec);
    static TMC_HF_EC_MODE convertEcMode(nn::pia::chat::EcMode mode);
    static int convertCid(unsigned short cid);
    static nn::pia::StationIndex convertStationIndex(nn::pia::StationIndex index);
    static nn::pia::Result createProtocol(nn::pia::transport::UnreliableProtocol** protocol, unsigned short* protocolId, unsigned char channel);
    static void destroyProtocol(nn::pia::transport::UnreliableProtocol** protocol, unsigned short* protocolId);
    static VCTCodec convertCodec(nn::pia::chat::Codec codec);
    static nn::pia::chat::Codec convertCodec(VCTCodec codec);
    static nn::pia::chat::EcMode convertEcMode(TMC_HF_EC_MODE mode);
};
#endif