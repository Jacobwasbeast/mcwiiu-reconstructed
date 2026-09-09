#include "vendor/__N_23_FFLiCompressUniform_cpp_56da346f.hpp"

void FFLiSwapEndianArrayImpl__tm__2_f__FPZ1ZUi_v(float* values, unsigned int count);
extern "C" void GX2Invalidate(unsigned int invalidateType, void* address, unsigned int size);

unsigned int __N_23_FFLiCompressUniform_cpp_56da346f::GetBlockAlignedSize(unsigned int value)
{
    return (value + 3U) & 0xfffffffcU;
}

void __N_23_FFLiCompressUniform_cpp_56da346f::SetupUniformBlock(float* values, unsigned int count, unsigned int size)
{
    FFLiSwapEndianArrayImpl__tm__2_f__FPZ1ZUi_v(values, count);
    GX2Invalidate(0x44U, values, size);
}
