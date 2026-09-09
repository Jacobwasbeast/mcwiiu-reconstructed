#include "nn/pia/chat/HandsFree.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"
#include "vendor/VCTAudio.hpp"
#include <cstring>

namespace nn { namespace pia { namespace chat {

HandsFree::HandsFree()
    : m_handle(0), m_ecMode(1), m_noiseSuppressorLevel(1),
      m_agcLevel(1), m_workspace(0), m_config()
{
}

HandsFree::~HandsFree()
{
    Finalize();
}

void HandsFree::Finalize()
{
    if (m_handle != 0)
    {
        TMC_HF_Close(m_handle);
        m_handle = 0;
    }
    if (m_workspace != 0)
    {
        pead::getArraySize(m_workspace);
        pead::freeToSeadHeap(m_workspace);
        m_workspace = 0;
    }
}

int HandsFree::Initialize(TMC_HF_EC_MODE ecMode,
                          int noiseSuppressorLevel, int agcLevel)
{
    if (m_handle != 0)
        return 0x10c02;
    if (static_cast<unsigned int>(ecMode) >= 3u ||
        static_cast<unsigned int>(noiseSuppressorLevel) >= 3u ||
        static_cast<unsigned int>(agcLevel) >= 5u)
        return 0x10c07;

    void* workspace = pead::allocateFromSeadHeap(
        0x251c0, common::HeapManager::GetHeap());
    if (workspace == 0)
        return 0x10c01;
    std::memset(workspace, 0, 0x251c0);
    m_ecMode = static_cast<int32_t>(ecMode);
    m_noiseSuppressorLevel = noiseSuppressorLevel;
    m_agcLevel = agcLevel;
    m_config.workspace = workspace;
    m_workspace = workspace;
    m_config.flags = 7;
    m_config.neutral = 0;
    m_config.echoCancellerMode = static_cast<int32_t>(ecMode);
    static const int16_t noiseLevels[3] = { 6, 10, 14 };
    m_config.noiseSuppressorLevel = noiseLevels[noiseSuppressorLevel];
    static const int16_t agcParameters[10] =
        { 0, 0, 10, -14, 14, -18, 17, -21, 19, -23 };
    m_config.agcParameterA = agcParameters[agcLevel * 2];
    m_config.agcParameterB = agcParameters[agcLevel * 2 + 1];
    m_config.inputMode = 0;
    m_config.outputMode = 0;
    m_config.error = 0;
    m_handle = TMC_HF_Init(&m_config);
    if (m_config.error != 0)
        return 0xc0b;
    return 0;
}

int HandsFree::Process(short* output, const short* inputLeft,
                       const short* inputRight, const short* inputReference)
{
    if (m_handle == 0)
        return 0x10c0c;
    if (output == 0 || inputLeft == 0 || inputRight == 0 ||
        inputReference == 0)
        return 0x10c07;

    uint32_t frames = VCT_GetAudioFrameLength() >> 3;
    uint32_t outputOffset = 0;
    uint32_t inputOffset = 0;
    while (frames-- != 0)
    {
        const uint32_t result = TMC_HF_Proc(
            m_handle, inputLeft + outputOffset, inputRight + inputOffset,
            inputReference + inputOffset, output + outputOffset);
        if (result != 0)
            return 0xc0b;
        outputOffset += 64;
        inputOffset += 256;
    }
    return 0;
}

}}}
