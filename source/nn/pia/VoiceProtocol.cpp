#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include <string.h>
#include "nn/pia/chat/HandsFree.hpp"
#include "platform/wiiu/pead.hpp"
#include "game/core/CriticalSection.hpp"
#include "vendor/VCTAudio.hpp"
#include <new>

extern unsigned int g_voiceAudioFrameLength = 0;
extern unsigned int g_voiceAudioDataSize = 0;
extern unsigned char g_voiceVadActivity = 0;
extern nn::pia::chat::VoiceProtocol* g_voiceProtocolInstance = 0;
OSFastMutex g_voiceProtocolMutex;

namespace nn
{
namespace pia
{
namespace chat
{



unsigned short ConvertID_Pia2VCT(nn::pia::StationIndex stationIndex)
{
    return static_cast<unsigned short>(stationIndex + 1);
}

VoiceProtocol::VoiceProtocol()
    : nn::pia::transport::Protocol(),
      m_secondaryState(0),
      m_vctSession(0),
      m_auxiliarySession(0),
      m_stationIndex(static_cast<StationIndex>(0xfd)),
      m_codec(4),
      m_bufferSize(0x90),
      m_maxPlayers(10),
      m_dataSize(0x20),
      m_sampleRate(0x10),
      m_criticalSection(-1),
      m_streamingState(0),
      m_monitoringBytesHigh(0),
      m_monitoringBytesLow(0),
      m_monitoringSpeechStart(0),
      m_monitoringSpeechEnd(0),
      m_monitoringTicksHigh(0),
      m_monitoringTicksLow(0),
      m_handsFree(0),
      m_pead(0)
{
}

VoiceProtocol::~VoiceProtocol()
{
    Finalize();
}
bool VoiceProtocol::IsInCommunication() const
{
    return m_stationIndex != 0xfd;
}

void VoiceProtocol::ClearStreaming()
{
    VCT_ClearStreaming(m_vctSession);
}

unsigned int VoiceProtocol::GetAudioFrameLength() const
{
    return g_voiceAudioFrameLength;
}

unsigned int VoiceProtocol::GetAudioDataSize() const
{
    return g_voiceAudioDataSize;
}

unsigned char VoiceProtocol::GetVadActivity() const
{
    return g_voiceVadActivity;
}

void VoiceProtocol::Finalize()
{
    g_voiceProtocolInstance = 0;
    destroyProtocols();
    freeProc();
}

Result VoiceProtocol::Initialize(const Setting& setting)
{
    if (static_cast<unsigned int>(setting.codec) >= 7 ||
        setting.bufferSize < 0x40 || setting.bufferSize > 0xc0 ||
        (setting.bufferSize & 7) != 0 || setting.dataSize == 0 ||
        setting.sampleRate < 2 || setting.sampleRate >= 0x11)
        return 0x10c07;

    if ((setting.codec == CODEC_5 || setting.codec == CODEC_6) &&
        (setting.bufferSize % 0x14) != 0)
        return 0x10c07;
    if (!nn::pia::chat::IsInitialized() ||
        !nn::pia::transport::IsInitialized())
        return 0x10c0c;
    if (g_voiceProtocolInstance != 0)
        return 0x10c02;
    if (nn::pia::transport::g_piaProtocolManager == 0)
        return 0x10c08;
    if (allocProc(setting.sampleRate) == 0)
        return 0x10c01;

    const Result setupResult = setupProtocols(
        static_cast<unsigned char>(setting.maxPlayers), setting.dataSize);
    if (setupResult != 0)
    {
        freeProc();
        return setupResult;
    }

    m_codec = setting.codec;
    m_bufferSize = setting.bufferSize;
    m_maxPlayers = static_cast<unsigned char>(setting.maxPlayers);
    m_dataSize = setting.dataSize;
    m_sampleRate = setting.sampleRate;
    g_voiceAudioFrameLength = setting.bufferSize;
    g_voiceAudioDataSize = setting.bufferSize << 4;
    g_voiceProtocolInstance = this;
    return 0;
}

int VoiceProtocol::calcAudioBufferSize(unsigned short sampleRate) const
{
    return static_cast<int>(sampleRate * 0x6e80u) - 0x6e80;
}

Result VoiceProtocol::allocProc(unsigned short sampleRate)
{
    m_vctSession = static_cast<VCTSession*>(pead::allocateFromSeadHeap(sampleRate * 0x3c));
    if (m_vctSession == 0) return 0;
    memset(m_vctSession, 0, sampleRate * 0x3c);
    m_auxiliarySession = pead::allocateFromSeadHeap(static_cast<unsigned int>(calcAudioBufferSize(sampleRate)), 0, 0x20);
    if (m_auxiliarySession == 0) { freeProc(); return 0; }
    void* handsFreeMemory = pead::allocateFromSeadHeap(sizeof(HandsFree));
    if (handsFreeMemory == 0) { freeProc(); return 0; }
    m_handsFree = new (handsFreeMemory) HandsFree();
    if (m_handsFree->Initialize(TMC_HF_EC_MODE_1, 1, 1) != 0) { freeProc(); return 0; }
    m_pead = pead::allocateFromSeadHeap(0xc00);
    if (m_pead == 0) { freeProc(); return 0; }
    memset(m_pead, 0, 0xc00);
    return 1;
}

void VoiceProtocol::freeProc()
{
    if (m_pead != 0)
    {
        pead::freeToSeadHeap(m_pead);
        m_pead = 0;
    }
    if (m_handsFree != 0)
    {
        m_handsFree->Finalize();
        m_handsFree->~HandsFree();
        pead::freeToSeadHeap(m_handsFree);
        m_handsFree = 0;
    }
    if (m_vctSession != 0)
    {
        pead::freeToSeadHeap(m_vctSession);
        m_vctSession = 0;
    }
    if (m_auxiliarySession != 0)
    {
        pead::freeToSeadHeap(m_auxiliarySession);
        m_auxiliarySession = 0;
    }
}

void VoiceProtocol::destroyProtocols()
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(
        reinterpret_cast<nn::pia::transport::UnreliableProtocol**>(&m_secondaryStatus),
        reinterpret_cast<unsigned short*>(&m_secondaryState));
}
Result VoiceProtocol::setupProtocols(unsigned char maxPlayers, unsigned int dataSize)
{
    Result result = __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol(
        reinterpret_cast<nn::pia::transport::UnreliableProtocol**>(&m_secondaryStatus),
        reinterpret_cast<unsigned short*>(&m_secondaryState), maxPlayers);
    if (result != 0) return result;
    const unsigned int initializeResult =
        reinterpret_cast<nn::pia::transport::UnreliableProtocol*>(m_secondaryStatus)->Initialize(dataSize);
    if (initializeResult != 0)
    {
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(
            reinterpret_cast<nn::pia::transport::UnreliableProtocol**>(&m_secondaryStatus),
            reinterpret_cast<unsigned short*>(&m_secondaryState));
        return static_cast<Result>(initializeResult);
    }
    return 0;
}

Result VoiceProtocol::receiveCore(void* pData, unsigned int uiSize, unsigned short* pCids)
{
    EnterCriticalSection(&g_voiceProtocolMutex);
    const Result result = VCT_ReceiveAudio(pData, uiSize, pCids);
    LeaveCriticalSection(&g_voiceProtocolMutex);
    return result;
}
Result VoiceProtocol::AddConferenceClient(StationIndex stationIndex)
{
    EnterCriticalSection(&g_voiceProtocolMutex);
    const int result = VCT_AddConferenceClient(ConvertID_Pia2VCT(stationIndex));
    Result mapped = 0xc0b;
    if (result == 0)
        mapped = 0;
    else if (result == -6 || result == -2)
        mapped = 0x10c08;
    else if (result == -1)
        mapped = 0x10c07;
    LeaveCriticalSection(&g_voiceProtocolMutex);
    return mapped;
}

Result VoiceProtocol::RemoveConferenceClient(StationIndex stationIndex)
{
    EnterCriticalSection(&g_voiceProtocolMutex);
    const int result = VCT_RemoveConferenceClient(ConvertID_Pia2VCT(stationIndex));
    Result mapped = 0xc0b;
    if (result == 0)
        mapped = 0;
    else if (result == -6 || result == -2)
        mapped = 0x10c08;
    else if (result == -1)
        mapped = 0x10c07;
    LeaveCriticalSection(&g_voiceProtocolMutex);
    return mapped;
}

Result VoiceProtocol::Send(const short* pData, unsigned int uiSamples,
                           const short* pLeft, const short* pRight)
{
    if (m_handsFree == 0 || m_pead == 0)
        return 0x10c0c;
    if (pData == 0 || pLeft == 0 || pRight == 0)
        return 0x10c07;

    const int processResult = m_handsFree->Process(
        const_cast<short*>(pData), static_cast<const short*>(m_pead),
        pLeft, pRight);
    if (processResult != 0)
        return processResult;
    const unsigned int scale = VCT_GetAudioScale(m_pead, uiSamples << 1);
    const int result = VCT_SendAudioEx(m_pead, scale);
    if (result == 0 || result == 0x2c0a)
        return 0;
    if (result == 0x10c08 || result == 0x10c07)
        return result;
    return 0xc0b;
}
Result VoiceProtocol::Receive(void* pData, unsigned int uiSize,
                              StationIndex* pStationIndex)
{
    if (pData == 0)
        return 0x10c07;
    unsigned short cids[16];
    const Result result = receiveCore(pData, uiSize, pStationIndex != 0 ? cids : 0);
    if (result != 0)
    {
        if (result == 0x2c09 || result == 0x10c07 || result == 0x10c08)
            return result;
        return 0xc0b;
    }
    if (pStationIndex != 0)
        *pStationIndex = static_cast<StationIndex>(
            __N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertCid(cids[0]));
    return 0;
}
Result VoiceProtocol::GetAudioInfo(StationIndex stationIndex, AudioInfo* audioInfo) const
{
    if (static_cast<unsigned short>(stationIndex) >= 0x20 || audioInfo == 0)
        return 0x10c07;
    VCTAudioInfo source;
    VCT_GetAudioInfoFromCid(static_cast<unsigned short>(stationIndex + 1), &source);
    memcpy(audioInfo, &source, sizeof(source));
    return 0;
}
bool VoiceProtocol::IsClientInConference(StationIndex stationIndex) const
{
    return VCT_OnConferenceClient(ConvertID_Pia2VCT(stationIndex)) != 0;
}
Result VoiceProtocol::AddConferenceClient(unsigned long long stationId)
{
    return AddConferenceClient(transport::Conv2StationIndex(stationId));
}

Result VoiceProtocol::RemoveConferenceClient(unsigned long long stationId)
{
    return RemoveConferenceClient(transport::Conv2StationIndex(stationId));
}

bool VoiceProtocol::IsClientInConference(unsigned long long stationId) const
{
    return IsClientInConference(transport::Conv2StationIndex(stationId));
}

Result VoiceProtocol::GetAudioInfo(unsigned long long stationId, AudioInfo* audioInfo) const
{
    return GetAudioInfo(transport::Conv2StationIndex(stationId), audioInfo);
}
}
}
}