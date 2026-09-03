#include "vendor/__N_18_FFLiSwapEndian_cpp_8c1df28d.hpp"

uint8_t __N_18_FFLiSwapEndian_cpp_8c1df28d::TypeToSize(FFLiSwapEndianType type)
{
    if (type < 3)
    {
        static const uint8_t sizes[3] = { 1, 2, 4 };
        return sizes[type];
    }
    return 0;
}
