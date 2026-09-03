#ifndef SOURCE_VENDOR___N_14_PEADENDIAN_CPP_EBA4357F_HPP
#define SOURCE_VENDOR___N_14_PEADENDIAN_CPP_EBA4357F_HPP

#include <stdint.h>

class __N_14_peadEndian_cpp_eba4357f
{
public:
    static unsigned int Swap16(unsigned short value);
    static unsigned int Swap32(unsigned int value);
    static unsigned long long Swap64(unsigned long long value);
    static void Swap8(unsigned char value);
    static void Null8(unsigned char value);
    static void Null16(unsigned short value);
    static void Null32(unsigned int value);
    static void Null64(unsigned long long value);
};

#endif
