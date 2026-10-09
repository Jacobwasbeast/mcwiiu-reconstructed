#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include <string.h>
#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/chat/HandsFree.hpp"
#include "platform/wiiu/pead.hpp"
#include "game/core/CriticalSection.hpp"
#include "vendor/VCTAudio.hpp"
#include "nn/pia/common/Memory.hpp"
#include <new>

extern nn::pia::chat::VoiceProtocol* g_voiceProtocolInstance = 0;
OSFastMutex g_voiceProtocolMutex;

namespace nn
{
namespace pia
{
namespace chat
{

VoiceProtocol::VoiceProtocol()
    : nn::pia::transport::Protocol(),
      m_unreliableProtocolId(0),
      m_pUnreliableProtocol(0),
      m_vctSession(0),
      m_audioBuffer(0),
      m_stationIndex(static_cast<StationIndex>(0xfd)),
      m_setting(),
      m_criticalSection(-1),
      m_dispatchCount(0),
      m_sentDataSize(0),
      m_receivedDataSize(0),
      m_startTick(0),
      m_handsFree(0),
      m_sendBuffer(0)
{
}

VoiceProtocol::~VoiceProtocol()
{
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
    return VCT_GetAudioFrameLength();
}

unsigned int VoiceProtocol::GetAudioDataSize() const
{
    return VCT_GetAudioDataSize();
}

bool VoiceProtocol::GetVadActivity() const
{
    return VCT_GetVADActivity();
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
    if (!allocProc(setting.sampleRate))
        return 0x10c01;

    const Result setupResult = setupProtocols(setting.maxPlayers, setting.dataSize);
    if (setupResult != 0)
    {
        freeProc();
        return setupResult;
    }

    m_setting = setting;
    g_voiceProtocolInstance = this;
    return 0;
}

int VoiceProtocol::calcAudioBufferSize(unsigned short sampleRate) const
{
    return static_cast<int>(sampleRate * 0x6e80u) - 0x6e80;
}

bool VoiceProtocol::allocProc(unsigned short sampleRate)
{
    m_vctSession = common::NewArray<VCTSession>(sampleRate);
    if (m_vctSession != 0)
    {
        m_audioBuffer = common::NewArray<unsigned char>(calcAudioBufferSize(sampleRate), 0x20);
        if (m_audioBuffer != 0)
        {
            m_handsFree = common::NewObj<HandsFree>();
            if (m_handsFree != 0)
            {
                if (m_handsFree->Initialize(__N_22_chat_VoiceProtocol_cpp_bf22a0fe::convertEcMode(EC_MODE_1), 1, 1) == 0)
                {
                    m_sendBuffer = common::NewArray<short>(cSendBufferSampleCount);
                    if (m_sendBuffer != 0)
                        return true;
                    m_handsFree->Finalize();
                }
                if (m_handsFree != 0)
                {
                    common::DeleteObj(m_handsFree);
                    m_handsFree = 0;
                }
            }
            if (m_audioBuffer != 0)
            {
                common::DeleteArray(m_audioBuffer);
                m_audioBuffer = 0;
            }
        }
        if (m_vctSession != 0)
        {
            common::DeleteArray(m_vctSession);
            m_vctSession = 0;
        }
    }
    return false;
}

void VoiceProtocol::freeProc()
{
    if (m_sendBuffer != 0)
    {
        common::DeleteArray(m_sendBuffer);
        m_sendBuffer = 0;
    }
    if (m_handsFree != 0)
    {
        m_handsFree->Finalize();
        if (m_handsFree != 0)
        {
            common::DeleteObj(m_handsFree);
            m_handsFree = 0;
        }
    }
    if (m_vctSession != 0)
    {
        common::DeleteArray(m_vctSession);
        m_vctSession = 0;
    }
    if (m_audioBuffer != 0)
    {
        common::DeleteArray(m_audioBuffer);
        m_audioBuffer = 0;
    }
}

void VoiceProtocol::destroyProtocols()
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(&m_pUnreliableProtocol, &m_unreliableProtocolId);
}
Result VoiceProtocol::setupProtocols(unsigned char maxPlayers, unsigned int dataSize)
{
    Result result = __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol(
        &m_pUnreliableProtocol, &m_unreliableProtocolId, maxPlayers);
    if (result != 0) return result;
    Result initializeResult =
        m_pUnreliableProtocol->Initialize(dataSize);
    if (initializeResult != 0)
    {
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(&m_pUnreliableProtocol, &m_unreliableProtocolId);
        return initializeResult;
    }
    return 0;
}

Result VoiceProtocol::receiveCore(void* pData, unsigned int uiSize, unsigned short* pCids)
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::lock_();
    const Result result = VCT_ReceiveAudio(pData, uiSize, pCids);
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
    return result;
}
Result VoiceProtocol::AddConferenceClient(StationIndex stationIndex)
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::lock_();
    switch (VCT_AddConferenceClient(ConvertID_Pia2VCT(stationIndex)))
    {
    case 0:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0;
    case -6:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c08;
    case -2:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c08;
    case -1:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c07;
    default:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0xc0b;
    }
}

Result VoiceProtocol::RemoveConferenceClient(StationIndex stationIndex)
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::lock_();
    switch (VCT_RemoveConferenceClient(ConvertID_Pia2VCT(stationIndex)))
    {
    case 0:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0;
    case -6:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c08;
    case -2:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c08;
    case -1:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0x10c07;
    default:
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::unlock_();
        return 0xc0b;
    }
}

Result VoiceProtocol::Send(const short* pData, unsigned int uiSamples,
                           const short* pLeft, const short* pRight)
{
    if (m_handsFree == 0 || m_sendBuffer == 0)
        return ResultNotInitialized();
    if (pData == 0 || pLeft == 0 || pRight == 0)
        return ResultInvalidArgument();

    const int processResult = m_handsFree->Process(
        const_cast<short*>(pData), m_sendBuffer,
        pLeft, pRight);
    if (processResult != 0)
        return processResult;
    const unsigned int scale = VCT_GetAudioScale(m_sendBuffer, uiSamples << 1);
    const int result = VCT_SendAudioEx(m_sendBuffer, scale);
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
        return ResultInvalidArgument();
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
        return ResultInvalidArgument();
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