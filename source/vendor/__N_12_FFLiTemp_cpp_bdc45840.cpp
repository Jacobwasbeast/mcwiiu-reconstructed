#include "vendor/__N_12_FFLiTemp_cpp_bdc45840.hpp"

void GX2Invalidate(unsigned int invalidateType, void* address, unsigned int size);

void __N_12_FFLiTemp_cpp_bdc45840::Invalidate()
{
    GX2Invalidate(8U, 0, 0xffffffffU);
}

bool __N_12_FFLiTemp_cpp_bdc45840::ShouldCallInvalidate(_GX2ShaderMode shaderMode)
{
    return shaderMode != 0;
}
