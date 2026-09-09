#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace chat {

enum TMC_HF_EC_MODE
{
    TMC_HF_EC_MODE_0 = 0,
    TMC_HF_EC_MODE_1 = 1,
    TMC_HF_EC_MODE_2 = 2
};

class HandsFree
{
public:
    HandsFree();
    ~HandsFree();

    void Finalize();
    int Initialize(TMC_HF_EC_MODE ecMode, int noiseSuppressorLevel,
                   int agcLevel);
    int Process(short* output, const short* inputLeft,
                const short* inputRight, const short* inputReference);

private:
    void* m_handle;
    int32_t m_ecMode;
    int32_t m_noiseSuppressorLevel;
    int32_t m_agcLevel;
    int32_t m_state;
    uint8_t m_config[32];
};

}}}

#endif
