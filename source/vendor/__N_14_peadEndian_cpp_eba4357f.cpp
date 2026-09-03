#include "vendor/__N_14_peadEndian_cpp_eba4357f.hpp"

unsigned int __N_14_peadEndian_cpp_eba4357f::Swap16(unsigned short value)
{
    return ((static_cast<unsigned int>(value) << 8) |
            (static_cast<unsigned int>(value) >> 8)) & 0xffffu;
}

unsigned int __N_14_peadEndian_cpp_eba4357f::Swap32(unsigned int value)
{
    return (value << 24) |
           ((value & 0xff00u) << 8) |
           (value >> 24) |
           ((value >> 8) & 0xff00u);
}

unsigned long long __N_14_peadEndian_cpp_eba4357f::Swap64(unsigned long long value)
{
    unsigned int high = static_cast<unsigned int>(value >> 32);
    unsigned int low = static_cast<unsigned int>(value);
    return (static_cast<unsigned long long>(Swap32(high)) << 32) |
           static_cast<unsigned long long>(Swap32(low));
}

void __N_14_peadEndian_cpp_eba4357f::Swap8(unsigned char value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null8(unsigned char value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null16(unsigned short value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null32(unsigned int value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null64(unsigned long long value)
{
}
