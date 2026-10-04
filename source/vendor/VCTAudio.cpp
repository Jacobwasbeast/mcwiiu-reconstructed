#include "vendor/VCTAudio.hpp"

#include "nn/pia/chat/AudioStream.hpp"
#include "nn/pia/chat/Decoder.hpp"
#include "vendor/VCTDropRecovery.hpp"
#include "vendor/VCTQuality.hpp"
#include "vendor/VCTQueue.hpp"
#include "vendor/__N_14_chat_Adpcm_cpp_f2b530af.hpp"

#include <string.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

namespace
{

const unsigned int VCT_STREAM_MAX = 15;
const unsigned int VCT_CONFERENCE_CID_MAX = 16;
const unsigned int VCT_AUDIO_SIGNATURE = 0x5f564354;
const unsigned int VCT_AUDIO_PACKET_TYPE = 0x40;
const unsigned int VCT_MODE_CONFERENCE = 2;
const unsigned int VCT_MODE_PEER = 1;
const unsigned int VCT_SESSION_STREAMING = 2;
const unsigned int VCT_SESSION_CONFERENCE_STREAMING = 3;
const unsigned int VCT_ADPCM_STATE_SIZE = 4;
const unsigned int VCT_TIMESTAMP_TICK_SCALE = 64;
const unsigned int VCT_PLAY_GUARD_MICROSECONDS = 100;

const int VCT_RESULT_OK = 0;
const int VCT_RESULT_FAILED = -1;
const int VCT_RESULT_INVALID_STATE = -2;
const int VCT_RESULT_FULL = -6;

const int VCT_RESULT_INVALID_ARGUMENT = 0x10c07;
const int VCT_RESULT_NOT_INITIALIZED = 0x10c08;
const int VCT_RESULT_NO_RECEIVE_DATA = 0x2c09;
const int VCT_RESULT_NO_SEND_TARGET = 0x2c0a;

const unsigned char g_vctPacketSizeTable[7][2] =
{
    { 8, 0 }, { 8, 0 }, { 2, 4 }, { 3, 4 }, { 4, 4 }, { 2, 8 }, { 4, 8 }
};

long long g_vctAudioFrameTicks = 0;
long long g_vctPlayTickTolerance = 0;
unsigned int g_vctStreamingSessionCount = 0;
VCTCodec g_vctCodec = VCT_CODEC_4;
unsigned int g_vctAudioFrameLength = 0x90;
unsigned int g_vctAudioDataSize = 0x900;
unsigned char g_vctVadResetPending = 0;
unsigned char g_vctVadEnabled = 1;
unsigned char g_vctVadDisableState = 0;
unsigned char g_vctSendPending = 0;
unsigned char g_vctAudioScaleSpecified = 0;
unsigned int g_vctTotalPlayTime = 0;
int g_vctHpfFeedForward = 0;
int g_vctHpfPrevious = 0;
unsigned int g_vctConferenceClientCount = 0;

nn::pia::chat::Adpcm::State g_vctAdpcmState[2];
unsigned short g_vctSendSequence = 0;
unsigned short g_vctUnknown63CA = 0;
unsigned short g_vctConferenceCids[VCT_CONFERENCE_CID_MAX] = { 0 };
const VCTSession* g_vctStreamingSessions[VCT_STREAM_MAX] = { 0 };
VCTAudioPacket g_vctSendPacketBuffer[2];
unsigned int g_vctSendAudioScale[2] = { 0 };
unsigned int g_vctSendState = 0;
unsigned int g_vctSendPacketCount = 0;
unsigned int g_vctSendCidCount = 0;
unsigned short g_vctSendCids[VCT_CONFERENCE_CID_MAX] = { 0 };
unsigned int g_vctSendUnknown26C = 0;
unsigned int g_vctSendUnknown270 = 0;
nn::pia::chat::AudioStream g_vctAudioStreams[VCT_STREAM_MAX];
short g_vctReceiveMixBuffer[0x600];
unsigned short g_vctLocalCid = 0;
VCTSession* g_vctFreeSessions = 0;
VCTSession* g_vctSessions = 0;

}

unsigned char g_vctInitialized = 0;
unsigned int g_vctMode = 0;

unsigned char VCT_IsInitialized()
{
    return g_vctInitialized;
}

unsigned int VCTi_GetAudioFrameLength()
{
    return g_vctAudioFrameLength;
}

unsigned int VCTi_GetAudioDataSize()
{
    return g_vctAudioDataSize;
}

unsigned int VCT_GetAudioFrameLength()
{
    return VCTi_GetAudioFrameLength();
}

unsigned int VCT_GetAudioDataSize()
{
    return VCTi_GetAudioDataSize();
}

void VCT_EnableVAD(bool bEnabled)
{
    g_vctVadEnabled = bEnabled ? 1 : 0;
    VCT_ResetVAD();
    if (!bEnabled)
    {
        g_vctVadDisableState = 1;
    }
}

bool vct_is_adpcm(VCTCodec codec)
{
    return codec >= VCT_CODEC_2 && codec <= VCT_CODEC_4;
}

bool vct_is_opus(VCTCodec codec)
{
    return codec == VCT_CODEC_5 || codec == VCT_CODEC_6;
}

unsigned int HPFFilter2(const void* pAudio, unsigned int uiSize)
{
    const short* pSamples = static_cast<const short*>(pAudio);
    unsigned long long ullEnergy = 0;
    unsigned int uiSamples = uiSize >> 1;
    const unsigned int uiCount = uiSamples;

    if (uiSamples != 0)
    {
        do
        {
            g_vctHpfPrevious =
                (*pSamples * 3743 + g_vctHpfFeedForward + g_vctHpfPrevious * 3390) >> 12;
            const int iSquare = g_vctHpfPrevious * g_vctHpfPrevious;
            g_vctHpfFeedForward = *pSamples * -3743;
            ++pSamples;
            ullEnergy += static_cast<unsigned long long>(static_cast<long long>(iSquare));
            --uiSamples;
        } while (uiSamples != 0);
    }

    return static_cast<unsigned int>(ullEnergy / uiCount);
}

unsigned int VCT_GetAudioScale(const void* pAudio, unsigned int uiSize)
{
    if (pAudio != 0 && uiSize != 0)
    {
        return HPFFilter2(pAudio, uiSize);
    }
    return 0;
}

int VCT_SendAudioEx(const void* pAudio, unsigned int uiAudioScale)
{
    const unsigned int uiDataSize = g_vctAudioDataSize;

    g_vctAudioScaleSpecified = 0;
    g_vctSendAudioScale[(g_vctSendPacketCount & 1) ^ 1] = uiAudioScale;

    if (!VCT_IsInitialized())
    {
        return VCT_RESULT_NOT_INITIALIZED;
    }
    if (uiDataSize != g_vctAudioDataSize)
    {
        return VCT_RESULT_INVALID_ARGUMENT;
    }
    if (g_vctStreamingSessionCount == 0)
    {
        return VCT_RESULT_NO_SEND_TARGET;
    }

    for (unsigned int uiSession = 0; uiSession < VCT_STREAM_MAX; ++uiSession)
    {
        const VCTSession* pSession = g_vctStreamingSessions[uiSession];
        if (pSession == 0 || pSession->m_State != VCT_SESSION_STREAMING)
        {
            continue;
        }

        VCTAudioPacket* pPacket = &g_vctSendPacketBuffer[(g_vctSendPacketCount & 1) ^ 1];
        pPacket->m_Signature = VCT_AUDIO_SIGNATURE;
        memcpy(pPacket->m_Data + (vct_is_adpcm(g_vctCodec) ? VCT_ADPCM_STATE_SIZE : 0),
               pAudio, uiDataSize);
        pPacket->m_Type = VCT_AUDIO_PACKET_TYPE;
        pPacket->m_Sequence = g_vctSendSequence;
        ++g_vctSendSequence;
        pPacket->m_Timestamp =
            static_cast<unsigned int>(OSGetTime() / VCT_TIMESTAMP_TICK_SCALE);
        g_vctSendPending = 1;
        ++g_vctSendPacketCount;
        return VCT_RESULT_OK;
    }

    return VCT_RESULT_NO_SEND_TARGET;
}

void vct_mix_audio(const short* pLeft, const short* pRight, short* pOut, unsigned int uiSize)
{
    for (unsigned int uiSample = 0; uiSample < (uiSize >> 1); ++uiSample)
    {
        int iMixed = static_cast<int>(pLeft[uiSample]) + static_cast<int>(pRight[uiSample]);
        if (iMixed >= 0x8000)
        {
            iMixed = 0x7fff;
        }
        else if (iMixed < -0x8000)
        {
            iMixed = -0x8000;
        }
        pOut[uiSample] = static_cast<short>(iMixed);
    }
}

int VCT_ReceiveAudio(void* pData, unsigned int uiSize, unsigned short* pCids)
{
    if (!VCT_IsInitialized())
    {
        return VCT_RESULT_NOT_INITIALIZED;
    }

    if (pCids != 0)
    {
        for (unsigned int uiSlot = 0; uiSlot < VCT_STREAM_MAX; ++uiSlot)
        {
            pCids[uiSlot] = 0;
        }
    }

    if (uiSize != VCTi_GetAudioDataSize())
    {
        memset(pData, 0, uiSize);
        return VCT_RESULT_INVALID_ARGUMENT;
    }

    if (g_vctStreamingSessionCount == 0)
    {
        memset(pData, 0, uiSize);
        return VCT_RESULT_NO_RECEIVE_DATA;
    }

    unsigned short usCid = 0;
    unsigned int uiStream = 0;
    unsigned int uiRemaining = VCT_STREAM_MAX;

    while (VCTi_ReceiveAudioData(pData, uiSize, uiStream, &usCid) == 0)
    {
        --uiRemaining;
        ++uiStream;
        if (uiRemaining == 0)
        {
            memset(pData, 0, uiSize);
            return VCT_RESULT_NO_RECEIVE_DATA;
        }
    }

    if (pCids != 0)
    {
        pCids[0] = usCid;
    }

    unsigned short* pNextCid = (pCids != 0) ? pCids + 1 : 0;
    for (unsigned int uiOther = uiStream + 1; uiOther < VCT_STREAM_MAX; ++uiOther)
    {
        if (VCTi_ReceiveAudioData(g_vctReceiveMixBuffer, uiSize, uiOther, &usCid) != 0)
        {
            if (pCids != 0)
            {
                *pNextCid = usCid;
                ++pNextCid;
            }
            vct_mix_audio(static_cast<const short*>(pData), g_vctReceiveMixBuffer,
                          static_cast<short*>(pData), uiSize);
        }
    }

    g_vctTotalPlayTime += uiSize >> 4;
    return VCT_RESULT_OK;
}

nn::pia::chat::AudioStream* VCTi_GetStreamFromCid(unsigned short usCid)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        if (g_vctAudioStreams[uiStream].m_Cid == usCid)
        {
            return &g_vctAudioStreams[uiStream];
        }
    }
    return 0;
}

unsigned int vct_handle_audio(unsigned short usCid, unsigned char* pData,
                               unsigned int uiDataSize, long long llReceiveTick)
{
    if (g_vctStreamingSessionCount == 0)
    {
        return 0;
    }

    nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }

    if (g_vctVadEnabled == 0)
    {
        pStream->UpdateSkew(llReceiveTick);
    }

    const VCTAudioPacket* pHeader = reinterpret_cast<const VCTAudioPacket*>(pData);
    const unsigned char ucType = pHeader->m_Type;
    if (ucType == 0x41)
    {
        pStream->m_SequenceUninitialized = 1;
        pStream->m_audioState = pHeader->m_Sequence;
        pStream->m_BaseSendTick = 0;
        return 0;
    }
    if (ucType != 0x40)
    {
        return 0;
    }

    const unsigned char ucInitialFlags = pData[5];
    const unsigned char ucCodec = static_cast<unsigned char>(ucInitialFlags & 0x0f);
    if (!VCT_IsValidCodec(static_cast<VCTCodec>(ucCodec)))
    {
        return 0;
    }

    pStream->m_Info.m_Unknown00 = ucCodec;

    const unsigned int uiMainSize = vct_get_packet_size(ucCodec);
    const bool bHasSpare = (ucInitialFlags & VCT_PACKET_FLAG_SPARE) != 0;
    const unsigned char* pSpare = 0;
    unsigned int uiSpareSize = 0;
    unsigned int uiPacketSize = uiMainSize;
    const unsigned int uiMainPayload = uiMainSize - 12;

    if (bHasSpare)
    {
        pSpare = pData + uiMainSize;
        const unsigned char ucSpareCodec = static_cast<unsigned char>(pSpare[5] & 0x0f);
        pData[uiMainSize + 5] = ucSpareCodec;
        if (!VCT_IsValidSpareCodec(static_cast<VCTCodec>(ucSpareCodec)))
        {
            return 0;
        }
        uiSpareSize = vct_get_packet_size(ucSpareCodec);
        uiPacketSize += uiSpareSize;
    }

    if (uiDataSize != uiPacketSize)
    {
        return 0;
    }

    if ((ucInitialFlags & 0x20) != 0)
    {
        VCTi_SetQualityAlertStatus(1);
    }
    else if (VCTi_GetQualityAlertStatus() == 1)
    {
        VCTi_SetQualityAlertClearStartTick(OSGetTime());
        VCTi_SetQualityAlertStatus(2);
    }

    unsigned int uiConferenceCid = 0;
    while (uiConferenceCid < VCT_CONFERENCE_CID_MAX &&
           g_vctConferenceCids[uiConferenceCid] != usCid)
    {
        ++uiConferenceCid;
    }
    if (uiConferenceCid == VCT_CONFERENCE_CID_MAX)
    {
        return 0;
    }

    _udp_packet* pPacket = vct_alloc_packet_buffer();
    if (pPacket == 0)
    {
        vct_flush_audio_queue(pStream->m_StreamIndex);
        nn::pia::chat::Decoder::Flush(pStream->m_StreamIndex);
        pPacket = vct_alloc_packet_buffer();
        if (pPacket == 0)
        {
            vct_flush_all_audio_queue();
            nn::pia::chat::Decoder::FlushAll();
            pPacket = vct_alloc_packet_buffer();
            if (pPacket == 0)
            {
                vct_trace_packet_buffer(0x4000000000ULL);
                return 0;
            }
        }
    }

    unsigned char* pBufferEnd = pPacket->m_Datagram +
                                VCT_AUDIO_FRAME_SAMPLE_MAX * sizeof(short);
    unsigned char* pFrameData = pBufferEnd - uiMainPayload;
    memcpy(pFrameData, pData + 12, uiDataSize - 12);
    pPacket->m_Codec = ucCodec;
    pPacket->m_StreamIndex = static_cast<unsigned char>(pStream->m_StreamIndex);
    pPacket->m_pFrame = reinterpret_cast<VCTAudioFrame*>(pFrameData);
    const unsigned char ucStoredFlags = pData[5];
    pPacket->m_Flags = ucStoredFlags;
    pPacket->m_Unknown28 = uiMainPayload;
    if ((ucStoredFlags & VCT_PACKET_FLAG_SPARE) != 0)
    {
        memcpy(pBufferEnd, pSpare, uiSpareSize);
    }

    pPacket->m_ReceiveTick = llReceiveTick;
    pPacket->m_Cid = usCid;
    pPacket->m_IsSpare = pData[5] & 0x80;
    pPacket->m_SendTick = static_cast<long long>(pHeader->m_Timestamp);
    pPacket->m_Sequence = pHeader->m_Sequence;

    ++pStream->m_Info.m_Unknown4C;
    const unsigned int uiSamplingPacketNum = VCT_GetQualityAlertSamplingPacketNum();
    if (uiSamplingPacketNum != 0)
    {
        const unsigned int uiSamplingCompare = VCT_GetQualityAlertSamplingPacketNum();
        if (uiSamplingCompare <= pStream->m_Info.m_Unknown4C)
        {
            const unsigned int uiExpiredCount = pStream->m_Info.m_ExpiredCount;
            const unsigned int uiSamplingDivisor = VCT_GetQualityAlertSamplingPacketNum();
            pStream->m_Info.m_ExpiredCount = 0;
            pStream->m_Info.m_Unknown4C = 0;
            pStream->m_Info.m_Unknown50 = (uiExpiredCount * 100U) / uiSamplingDivisor;
        }
    }

    if (pStream->CalcJitterAndSkew(pPacket) == 0)
    {
        vct_free_packet_buffer(pPacket);
        return 0;
    }

    nn::pia::chat::Decoder::Insert(pPacket);
    return 1;
}

unsigned int VCT_GetDropCount(unsigned short usCid)
{
    const nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }
    return pStream->m_Info.m_DropCount;
}

unsigned int VCT_GetDropFrequency(unsigned short usCid)
{
    const nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        return 0;
    }

    const unsigned int uiDropCount = VCT_GetDropCount(usCid);
    const unsigned long long ullNow = static_cast<unsigned long long>(OSGetTime());
    const unsigned long long ullElapsed = ullNow - pStream->m_DropMeasureStartTick;
    const unsigned int uiTimerClock = OSGetSystemInfo()->busClockSpeed >> 2;
    const unsigned int uiSeconds =
        static_cast<unsigned int>(ullElapsed * 1000 / uiTimerClock / 1000);

    return uiDropCount * 60 / uiSeconds;
}

void VCT_GetAudioInfoFromCid(unsigned short usCid, VCTAudioInfo* pAudioInfo)
{
    nn::pia::chat::AudioStream* pStream = VCTi_GetStreamFromCid(usCid);
    if (pStream == 0)
    {
        memset(pAudioInfo, 0, sizeof(VCTAudioInfo));
        return;
    }

    pStream->m_DropFrequency = VCT_GetDropFrequency(usCid);
    memcpy(pAudioInfo, &pStream->m_Info, sizeof(VCTAudioInfo));
}

VCTSession* vct_find_session(unsigned short usCid)
{
    VCTSession* pSession = g_vctSessions;
    while (pSession != 0 && (pSession->m_Mode == 0 || pSession->m_Cid != usCid))
    {
        pSession = pSession->m_Next;
    }
    return pSession;
}

VCTSession* VCTi_CreateSession(unsigned short usCid)
{
    VCTSession* pSession = g_vctFreeSessions;
    if (usCid == g_vctLocalCid || pSession == 0)
    {
        return 0;
    }

    g_vctFreeSessions = pSession->m_Next;
    memset(pSession, 0, sizeof(VCTSession));
    pSession->m_State = 0;
    pSession->m_Mode = g_vctMode;
    pSession->m_Cid = usCid;
    pSession->m_Next = g_vctSessions;
    g_vctSessions = pSession;
    return pSession;
}

int VCT_DeleteSession(VCTSession* pSession)
{
    if (pSession == 0)
    {
        return 0;
    }
    if (g_vctMode == VCT_MODE_PEER)
    {
        return 1;
    }

    VCTSession* pPrevious = 0;
    for (VCTSession* pCurrent = g_vctSessions; pCurrent != 0; pCurrent = pCurrent->m_Next)
    {
        if (pCurrent == pSession)
        {
            pSession->m_Mode = 0;
            if (pPrevious != 0)
            {
                pPrevious->m_Next = pSession->m_Next;
            }
            else
            {
                g_vctSessions = pCurrent->m_Next;
            }
            pSession->m_Next = g_vctFreeSessions;
            g_vctFreeSessions = pSession;
            return 1;
        }
        pPrevious = pCurrent;
    }

    return 0;
}

void VCT_ClearStreaming(const VCTSession*)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        nn::pia::chat::AudioStream& stream = g_vctAudioStreams[uiStream];
        if (stream.m_PreviousPacket != 0)
        {
            vct_free_packet_buffer(stream.m_PreviousPacket);
            stream.m_PreviousPacket = 0;
        }
        vct_flush_audio_queue(uiStream);
        nn::pia::chat::Decoder::Flush(uiStream);
    }
}

int VCT_AddConferenceClient(unsigned short usCid)
{
    if (g_vctMode != VCT_MODE_CONFERENCE)
    {
        return VCT_RESULT_INVALID_STATE;
    }
    if (g_vctConferenceClientCount == VCT_STREAM_MAX)
    {
        return VCT_RESULT_FULL;
    }

    if (usCid != g_vctLocalCid)
    {
        if (vct_find_session(usCid) != 0)
        {
            return VCT_RESULT_OK;
        }

        VCTSession* pSession = VCTi_CreateSession(usCid);
        if (pSession != 0)
        {
            pSession->m_Cid = usCid;
            pSession->m_OwnerCid = usCid;
            pSession->m_ConferenceCids[0] = usCid;
            pSession->m_State = VCT_SESSION_STREAMING;
            for (unsigned int uiEntry = 1; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                pSession->m_ConferenceCids[uiEntry] = 0;
            }
            VCT_StartStreaming(pSession);
            ++g_vctConferenceClientCount;
            return VCT_RESULT_OK;
        }
    }

    return VCT_RESULT_FAILED;
}

int VCT_RemoveConferenceClient(unsigned short usCid)
{
    if (g_vctMode != VCT_MODE_CONFERENCE)
    {
        return VCT_RESULT_INVALID_STATE;
    }

    VCTSession* pSession = vct_find_session(usCid);
    if (pSession == 0)
    {
        return VCT_RESULT_FAILED;
    }

    VCT_StopStreaming(pSession);
    VCT_DeleteSession(pSession);
    --g_vctConferenceClientCount;
    return VCT_RESULT_OK;
}

int VCT_OnConferenceClient(unsigned short usCid)
{
    if (g_vctMode == VCT_MODE_CONFERENCE && vct_find_session(usCid) != 0)
    {
        return 1;
    }
    return 0;
}

long long VCTi_GetAudioFrameTicks()
{
    return g_vctAudioFrameTicks;
}

void init_adpcm_state()
{
    g_vctAdpcmState[0].sample = 0;
    g_vctAdpcmState[0].index = 0;
    g_vctAdpcmState[1].sample = 0;
    g_vctAdpcmState[1].index = 0;
}

int vct_get_packet_size(unsigned char ucCodec)
{
    if (ucCodec < 5)
    {
        return g_vctPacketSizeTable[ucCodec][0] * g_vctAudioFrameLength +
               g_vctPacketSizeTable[ucCodec][1] + 12;
    }
    if (ucCodec > 6)
    {
        return 0;
    }
    return g_vctPacketSizeTable[ucCodec][0] * g_vctAudioFrameLength +
           g_vctPacketSizeTable[ucCodec][1] * (g_vctAudioFrameLength / 0x14) + 12;
}

nn::pia::chat::AudioStream* vct_get_audio_stream(unsigned int uiStreamIndex)
{
    if (uiStreamIndex >= VCT_STREAM_MAX)
    {
        return 0;
    }
    return &g_vctAudioStreams[uiStreamIndex];
}

int VCTi_ReceiveAudioData(void* pData, unsigned int uiSize, unsigned int uiStreamIndex,
                          unsigned short* pCid)
{
    nn::pia::chat::AudioStream* pStream = &g_vctAudioStreams[uiStreamIndex];
    const unsigned short usStreamCid = static_cast<unsigned short>(pStream->m_Cid);
    bool bFirstPacket = true;

    int iQueued = vct_count_audio_queue(uiStreamIndex);
    if (iQueued != 0)
    {
        do
        {
            const long long llNow = OSGetTime();
            _udp_packet* pPacket = vct_top_audio_queue(uiStreamIndex);
            if (pPacket == 0)
            {
                pStream->m_Info.m_QueuedPacketCount = static_cast<unsigned int>(iQueued);
                return 0;
            }

            const int iArrivalSampleCount = pStream->m_ArrivalSampleCount;
            const int iArrivalDelay =
                static_cast<int>(llNow) - static_cast<int>(pPacket->m_PlayTick);
            pStream->m_LastArrivalDelay = iArrivalDelay;
            if (pStream->m_MaxArrivalDelay < iArrivalDelay)
            {
                pStream->m_MaxArrivalDelay = iArrivalDelay;
            }
            pStream->m_ArrivalSampleCount = iArrivalSampleCount + 1;
            pStream->m_AverageArrivalDelay =
                (pStream->m_AverageArrivalDelay * iArrivalSampleCount + iArrivalDelay) /
                (iArrivalSampleCount + 1);

            const unsigned int uiLastSequence = pStream->m_LastSequence;
            const unsigned int uiSequence = pPacket->m_Sequence;
            if (uiLastSequence + 1 != uiSequence && uiLastSequence != 0 &&
                pPacket->m_IsSpare == 0)
            {
                if (uiSequence < uiLastSequence)
                {
                    ++pStream->m_Info.m_OutOfOrderCount;
                }
                else if (uiLastSequence < uiSequence)
                {
                    ++pStream->m_Info.m_DropCount;
                }
                else
                {
                    ++pStream->m_Info.m_DuplicateCount;
                }
            }

            const unsigned long long ullGuard = static_cast<unsigned long long>(OSGetSystemInfo()->busClockSpeed >> 2) *
                                                VCT_PLAY_GUARD_MICROSECONDS / 1000000;
            if (static_cast<unsigned long long>(llNow) + ullGuard <
                static_cast<unsigned long long>(pPacket->m_PlayTick))
            {
                ++pStream->m_Info.m_TooEarlyCount;
                if (pPacket->m_IsSpare != 0)
                {
                    return 0;
                }
                if (vct_recover_drop_using_spare(pStream, pPacket, pData, uiSize) == 0)
                {
                    break;
                }
                *pCid = usStreamCid;
                if (pStream->m_PreviousPacket != 0)
                {
                    vct_free_packet_buffer(pStream->m_PreviousPacket);
                    pStream->m_PreviousPacket = 0;
                }
                pStream->m_Info.m_ContinuousDropCount = 0;
                return 1;
            }

            if (llNow > pPacket->m_PlayTick + g_vctPlayTickTolerance)
            {
                ++pStream->m_Info.m_LateCount;
                ++pStream->m_Info.m_ExpiredCount;
                pStream->m_LastSequence = pPacket->m_Sequence;
                iQueued = pStream->SavePreviousBuffer(pPacket);
                pPacket = vct_top_audio_queue(uiStreamIndex);
                bFirstPacket = false;
                continue;
            }

            pStream->m_LastSequence = pPacket->m_Sequence;

            if (bFirstPacket && pStream->m_SkewClampCount != 0)
            {
                iQueued = pStream->SavePreviousBuffer(pPacket);
                pPacket = vct_top_audio_queue(uiStreamIndex);
                pStream->m_SkewClampCount = pStream->m_SkewClampCount - 1;
                continue;
            }

            const int iPreviousDelay = static_cast<int>(pStream->m_PreviousArrivalDelay);
            const int iReceiveDelay =
                static_cast<int>(llNow) - static_cast<int>(pPacket->m_ReceiveTick);
            pStream->m_PreviousArrivalDelay = static_cast<unsigned int>(iReceiveDelay);
            const int iSmoothedDelay = iReceiveDelay + (-iPreviousDelay >> 4);
            pStream->m_SmoothedArrivalDelay = static_cast<unsigned int>(iSmoothedDelay);
            pStream->m_Info.m_Latency = static_cast<unsigned int>(iSmoothedDelay);

            memcpy(pData, pPacket->m_pFrame, uiSize);
            pStream->SavePreviousBuffer(pPacket);

            if (pStream->m_DropRecovered != 0)
            {
                vct_filter_fade_in(static_cast<const short*>(pData), static_cast<short*>(pData),
                                   uiSize >> 1, VCT_GetDropRecoverInterpolateSampleNum(),
                                   pStream->m_LastRecoveredSample);
                pStream->m_DropRecovered = 0;
            }

            *pCid = pPacket->m_Cid;
            pStream->m_Info.m_ContinuousDropCount = 0;
            return 1;
        }
        while (iQueued != 0);
    }

    const int iRecovered = vct_recover_drop_stream(pStream, pData, uiSize);
    if (iRecovered != 0)
    {
        *pCid = usStreamCid;
    }
    return iRecovered;
}

int VCT_StartStreaming(const VCTSession* pSession)
{
    if (pSession == 0 || g_vctStreamingSessionCount == VCT_STREAM_MAX ||
        (pSession->m_State != VCT_SESSION_STREAMING &&
         pSession->m_State != VCT_SESSION_CONFERENCE_STREAMING))
    {
        return 0;
    }

    for (unsigned int uiSlot = 0; uiSlot < VCT_STREAM_MAX; ++uiSlot)
    {
        if (g_vctStreamingSessions[uiSlot] == pSession)
        {
            return 1;
        }
    }

    unsigned int uiStream = 0;
    while (uiStream < VCT_STREAM_MAX && g_vctStreamingSessions[uiStream] != 0)
    {
        ++uiStream;
    }
    if (uiStream < VCT_STREAM_MAX)
    {
        g_vctStreamingSessions[uiStream] = pSession;
    }
    ++g_vctStreamingSessionCount;

    for (unsigned int uiIndex = 0; uiIndex < VCT_CONFERENCE_CID_MAX; ++uiIndex)
    {
        const unsigned short usCid = pSession->m_ConferenceCids[uiIndex];
        if (usCid == 0)
        {
            continue;
        }

        unsigned int uiEntry = 0;
        while (uiEntry < VCT_CONFERENCE_CID_MAX && g_vctConferenceCids[uiEntry] != usCid)
        {
            ++uiEntry;
        }
        if (uiEntry == VCT_CONFERENCE_CID_MAX)
        {
            unsigned short* pSlot = g_vctConferenceCids;
            while (*pSlot != 0)
            {
                ++pSlot;
            }
            *pSlot = usCid;
        }
    }

    g_vctAudioStreams[uiStream].Init(uiStream, pSession->m_OwnerCid);
    vct_init_audio_queue(uiStream);

    if (pSession->m_State == VCT_SESSION_STREAMING && g_vctStreamingSessionCount == 1)
    {
        VCT_ResetVAD();
        g_vctSendSequence = static_cast<unsigned short>(OSGetTime());
        init_adpcm_state();
        g_vctVadResetPending = 1;
        g_vctSendState = 0;
        g_vctSendPacketCount = 0;
        g_vctSendCidCount = 0;
        for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
        {
            g_vctSendCids[uiEntry] = 0;
        }
        g_vctSendUnknown26C = 0;
        g_vctSendUnknown270 = 0;
    }

    return 1;
}

void VCT_StopStreaming(const VCTSession* pSession)
{
    for (unsigned int uiStream = 0; uiStream < VCT_STREAM_MAX; ++uiStream)
    {
        if (g_vctStreamingSessions[uiStream] != pSession)
        {
            continue;
        }

        g_vctStreamingSessions[uiStream] = 0;
        --g_vctStreamingSessionCount;
        g_vctAudioStreams[uiStream].Reset();
        vct_flush_audio_queue(uiStream);
        nn::pia::chat::Decoder::Flush(uiStream);

        for (unsigned int uiIndex = 0; uiIndex < VCT_CONFERENCE_CID_MAX; ++uiIndex)
        {
            const unsigned short usCid = pSession->m_ConferenceCids[uiIndex];
            if (usCid == 0)
            {
                continue;
            }
            for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
            {
                if (g_vctConferenceCids[uiEntry] == usCid)
                {
                    g_vctConferenceCids[uiEntry] = 0;
                    break;
                }
            }
        }
    }

    if (g_vctStreamingSessionCount == 0)
    {
        g_vctVadResetPending = 1;
        VCT_ResetVAD();
        for (unsigned int uiEntry = 0; uiEntry < VCT_CONFERENCE_CID_MAX; ++uiEntry)
        {
            g_vctConferenceCids[uiEntry] = 0;
        }
    }
}
