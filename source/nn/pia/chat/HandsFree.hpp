#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_CHAT_HANDSFREE_HPP

#include <stdint.h>
#include "vendor/__N_18_chat_HandsFree_cpp_120265b3.hpp"

namespace nn { namespace pia { namespace chat {

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
    struct TmcConfig
    {
        void* workspace;
        uint16_t flags;
        uint16_t neutral;
        int32_t echoCancellerMode;
        int16_t noiseSuppressorLevel;
        int16_t agcParameterA;
        int16_t agcParameterB;
        int32_t inputMode;
        int32_t outputMode;
        int32_t error;
    };

    void* m_handle;
    int32_t m_ecMode;
    int32_t m_noiseSuppressorLevel;
    int32_t m_agcLevel;
    void* m_workspace;
    TmcConfig m_config;
};

}}}

#endif
