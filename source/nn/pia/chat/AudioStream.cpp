#include "nn/pia/chat/AudioStream.hpp"

#include "vendor/VCTQueue.hpp"

#include <string.h>
#include <coreinit/systeminfo.h>

namespace
{
    long long s_skewCheckInterval = 0;
    int s_skewCheckInitialized = 0;
    long long s_skewLimit = 0;
    long long s_jitterOffsetLimit = 0;
    long long s_baseJitterOffset = 0;
    unsigned int s_stableFrameThreshold = 0;

    unsigned int timerClock()
    {
        return OSGetSystemInfo()->busClockSpeed >> 2;
    }
}

namespace nn { namespace pia { namespace chat {

AudioStream::AudioStream()
{
    memset(&m_Info, 0, sizeof(m_Info));
    m_PreviousArrivalDelay = 0;
    m_SmoothedArrivalDelay = 0;
    m_StreamIndex = 0;
    m_Unknown6C = 0;
    m_Cid = 0;
    m_PreviousPacket = 0;
    m_BaseReceiveTick = 0;
    m_BaseSendTick = 0;
    m_Unknown88 = 0;
    m_LastQueuedSequence = 0;
    m_LastSequence = 0;
    m_SequenceBase = 0;
    m_SequenceUninitialized = 0;
    m_PreviousRawSequence = 0;
    m_ExpectedRawSequence = 0;
    m_SkewSampleCount = 0;
    m_SkewClampCount = 0;
    m_SkewAverage = 0;
    m_SkewLatest = 0;
    m_DropMeasureStartTick = 0;
    m_DropFrequency = 0;
    m_JitterSampleCount = 0;
    m_SkewSampleTimestamp = 0;
    m_JitterAverage = 0;
    m_JitterBufferTicks = 0;
    m_JitterBufferMinimum = 0;
    m_PreviousArrivalTick = 0;
    m_JitterHoldCount = 0;
    m_StableFrameCount = 0;
    m_LastRecoveredSample = 0;
    m_DropRecovered = 0;
    m_LastArrivalDelay = 0;
    m_MaxArrivalDelay = 0;
    m_AverageArrivalDelay = 0;
    m_ArrivalSampleCount = 0;
    m_SkewCheckTick = 0;
}

AudioStream::~AudioStream()
{
}

void AudioStream::InitEx()
{
    m_Info.m_Unknown00 = 0;
    m_Info.m_Unknown04 = 0;
    m_Info.m_LastSequence = 0;
    m_Info.m_Unknown0C = 0;
    m_Info.m_JitterOffset = s_baseJitterOffset;
    m_Info.m_ContinuousDropCount = 0;
    m_Info.m_ExpiredCount = 0;
    m_Info.m_Unknown4C = 0;
    m_Info.m_Unknown50 = 0;
    m_PreviousArrivalDelay = 0;
    m_SmoothedArrivalDelay = 0;
    m_Unknown6C = 0;
    m_PreviousPacket = 0;
    m_BaseReceiveTick = 0;
    m_BaseSendTick = 0;
    m_Unknown88 = 0;
    m_LastQueuedSequence = 0;
    m_LastSequence = 0;
    m_SequenceUninitialized = 1;
    m_SkewSampleCount = 0;
    m_SkewClampCount = 0;
    m_JitterSampleCount = 0;
    m_SkewSampleTimestamp = 0;
    m_JitterAverage = 0;
    m_JitterBufferTicks = s_baseJitterOffset;
    m_JitterBufferMinimum = 0;
    m_PreviousArrivalTick = 0;
    m_JitterHoldCount = 0;
    m_StableFrameCount = 0;
    m_LastRecoveredSample = 0;
    m_DropRecovered = 0;
}

void AudioStream::Init(unsigned int uiStreamIndex, int iCid)
{
    InitEx();
    m_StreamIndex = uiStreamIndex;
    m_Cid = static_cast<unsigned int>(iCid);
}

void AudioStream::Reset()
{
    if (m_PreviousPacket != 0)
    {
        vct_free_packet_buffer(m_PreviousPacket);
        m_PreviousPacket = 0;
    }
    m_Cid = 0xffffffff;
}

int AudioStream::SavePreviousBuffer(_udp_packet* pPacket)
{
    if (m_PreviousPacket != 0)
    {
        vct_free_packet_buffer(m_PreviousPacket);
    }
    m_PreviousPacket = pPacket;
    return vct_pop_audio_queue(m_StreamIndex);
}

void AudioStream::InitSequence(unsigned short usSequence)
{
    m_PreviousRawSequence = usSequence;
    m_ExpectedRawSequence = 0x10001;
    m_SequenceUninitialized = 0;
    m_SequenceBase = 0;
}

int AudioStream::UpdateSequence(_udp_packet* pPacket, unsigned short usSequence)
{
    unsigned int uiDistance = (usSequence - m_PreviousRawSequence) & 0xffff;
    unsigned int uiExtended;

    if (uiDistance < 3000)
    {
        if (usSequence < m_PreviousRawSequence)
        {
            m_SequenceBase = m_SequenceBase + 0x10000;
        }
        uiExtended = usSequence + m_SequenceBase;
        m_PreviousRawSequence = usSequence;
    }
    else if (uiDistance < 0xff9d)
    {
        if (usSequence != m_ExpectedRawSequence)
        {
            m_ExpectedRawSequence = (usSequence + 1) & 0xffff;
            return 0;
        }
        InitSequence(usSequence);
        uiExtended = usSequence + m_SequenceBase;
        m_LastQueuedSequence = (usSequence - 1) & 0xffff;
    }
    else
    {
        uiExtended = usSequence + m_SequenceBase;
        if (m_LastQueuedSequence + 0xff9c < uiExtended)
        {
            pPacket->m_Sequence = uiExtended - 0x10000;
            return 1;
        }
    }

    pPacket->m_Sequence = uiExtended;
    return 1;
}

void AudioStream::UpdateSkew(long long llTick)
{
    if (s_skewCheckInitialized == 0)
    {
        s_skewCheckInitialized = 1;
        s_skewCheckInterval =
            static_cast<long long>(static_cast<unsigned long long>(timerClock()) * 900);
    }

    if (m_SkewCheckTick != 0)
    {
        if (llTick - m_SkewCheckTick <= s_skewCheckInterval)
        {
            return;
        }
        m_BaseSendTick = 0;
        m_SkewSampleCount = 1;
    }

    m_SkewCheckTick = llTick;
}

void AudioStream::MakeStreamParams(unsigned int uiFrameMilliseconds)
{
    const long long llFrameTicks = VCTi_GetAudioFrameTicks();

    s_skewLimit = static_cast<long long>(
        static_cast<unsigned long long>(uiFrameMilliseconds - 2) * timerClock() / 1000);

    const long long llHeadroom = static_cast<long long>(timerClock()) - s_skewLimit;
    s_jitterOffsetLimit = llHeadroom -
        static_cast<long long>(static_cast<unsigned long long>(timerClock()) * 30 / 1000);

    s_stableFrameThreshold = (1000 / uiFrameMilliseconds) * 5;

    s_baseJitterOffset = llFrameTicks +
        static_cast<long long>(static_cast<unsigned long long>(timerClock()) * 0x53 / 1000);
}

}}}
