#include "vendor/__N_12_FFLiTemp_cpp_bdc45840.hpp"

#include "platform/wiiu/FFL.hpp"

void __N_12_FFLiTemp_cpp_bdc45840::Invalidate()
{
    FFLiInvalidate(8U, 0, 0xffffffffU);
}

bool __N_12_FFLiTemp_cpp_bdc45840::ShouldCallInvalidate(_GX2ShaderMode shaderMode)
{
    return shaderMode != 0;
}
