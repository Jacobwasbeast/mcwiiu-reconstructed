#include "nn/pia/chat/HandsFree.hpp"

extern int32_t TMC_HF_Init(void* config);
extern uint32_t TMC_HF_Proc(void* handle, short* output,
                            const short* inputLeft, const short* inputRight,
                            const short* inputReference);
extern uint32_t TMC_HF_Close(void* handle);
extern uint32_t VCT_GetAudioFrameLength();

namespace nn { namespace pia { namespace chat {

HandsFree::HandsFree()
    : m_handle(0), m_ecMode(1), m_noiseSuppressorLevel(1),
      m_agcLevel(1), m_state(0), m_config()
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
    m_state = 0;
}

int HandsFree::Initialize(TMC_HF_EC_MODE ecMode,
                          int noiseSuppressorLevel, int agcLevel)
{
    if (m_handle != 0)
        return 0x10c07;
    if (static_cast<unsigned int>(ecMode) >= 3u ||
        static_cast<unsigned int>(noiseSuppressorLevel) >= 3u ||
        static_cast<unsigned int>(agcLevel) >= 5u)
        return 0x10c07;

    m_ecMode = static_cast<int32_t>(ecMode);
    m_noiseSuppressorLevel = noiseSuppressorLevel;
    m_agcLevel = agcLevel;
    m_state = 0;
    const int32_t result = TMC_HF_Init(m_config);
    if (result != 0)
        return result;
    m_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(result));
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

    uint32_t frames = VCT_GetAudioFrameLength();
    uint32_t outputOffset = 0;
    uint32_t inputOffset = 0;
    while (frames-- != 0)
    {
        const uint32_t result = TMC_HF_Proc(
            m_handle, output + outputOffset, inputLeft + inputOffset,
            inputRight + inputOffset, inputReference + inputOffset);
        if (result != 0)
            return static_cast<int>(result);
        outputOffset += 128;
        inputOffset += 512;
    }
    return 0;
}

}}}
