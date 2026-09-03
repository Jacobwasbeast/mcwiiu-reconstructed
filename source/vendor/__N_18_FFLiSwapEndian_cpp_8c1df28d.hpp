#ifndef SOURCE_VENDOR___N_18_FFLISWAPENDIAN_CPP_8C1DF28D_HPP
#define SOURCE_VENDOR___N_18_FFLISWAPENDIAN_CPP_8C1DF28D_HPP

#include <stdint.h>

enum FFLiSwapEndianType
{
    FFLiSwapEndianTypeByte = 0,
    FFLiSwapEndianTypeHalfword = 1,
    FFLiSwapEndianTypeWord = 2
};

class __N_18_FFLiSwapEndian_cpp_8c1df28d
{
public:
    static uint8_t TypeToSize(FFLiSwapEndianType type);
};

#endif
