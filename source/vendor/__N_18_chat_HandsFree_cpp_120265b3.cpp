#include "vendor/__N_18_chat_HandsFree_cpp_120265b3.hpp"

static unsigned int functionalBits = 0;


bool __N_18_chat_HandsFree_cpp_120265b3::isValidEcMode(TMC_HF_EC_MODE mode)
{
    return static_cast<unsigned int>(mode) < 3u;
}

bool __N_18_chat_HandsFree_cpp_120265b3::isValidNsLevel(int level)
{
    return level < 3;
}

bool __N_18_chat_HandsFree_cpp_120265b3::isValidAgcLevel(int level)
{
    return level < 5;
}

void __N_18_chat_HandsFree_cpp_120265b3::updateFunctionalBits(unsigned int bits, bool enabled)
{
    if (enabled)
    {
        functionalBits &= ~bits;
    }
    else
    {
        functionalBits |= bits;
    }
}
