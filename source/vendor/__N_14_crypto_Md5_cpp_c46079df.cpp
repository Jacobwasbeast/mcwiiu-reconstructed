#include "vendor/__N_14_crypto_Md5_cpp_c46079df.hpp"

static uint32_t SwapBytes(uint32_t value)
{
    return (value << 24) | ((value & 0x0000ff00U) << 8) | (value >> 24) | ((value & 0x00ff0000U) >> 8);
}

static uint32_t RotateLeft(uint32_t value, uint32_t shift)
{
    shift &= 31U;
    return (value << shift) | (value >> ((32U - shift) & 31U));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound1(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift, uint32_t constant)
{
    uint32_t value = a + ((b & c) | (d & ~b)) + SwapBytes(messageWord) + constant;
    return (int32_t)(b + RotateLeft(value, shift));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound2(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift, uint32_t constant)
{
    uint32_t value = a + ((b & d) | (c & ~d)) + SwapBytes(messageWord) + constant;
    return (int32_t)(b + RotateLeft(value, shift));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound4(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift, uint32_t constant)
{
    uint32_t value = a + (c ^ (b | ~d)) + SwapBytes(messageWord) + constant;
    return (int32_t)(b + RotateLeft(value, shift));
}
