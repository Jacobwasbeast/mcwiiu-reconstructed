#ifndef SOURCE_VENDOR___N_18_CHAT_HANDSFREE_CPP_120265B3_HPP
#define SOURCE_VENDOR___N_18_CHAT_HANDSFREE_CPP_120265B3_HPP

#include "vendor/TmcHandsFree.hpp"

enum TMC_HF_EC_MODE
{
    TMC_HF_EC_MODE_0 = 0,
    TMC_HF_EC_MODE_1 = 1,
    TMC_HF_EC_MODE_2 = 2
};

class __N_18_chat_HandsFree_cpp_120265b3
{
public:
    bool isValidEcMode(TMC_HF_EC_MODE mode);
    bool isValidNsLevel(int level);
    bool isValidAgcLevel(int level);
    void updateFunctionalBits(unsigned int bits, bool enabled);
};

#endif
