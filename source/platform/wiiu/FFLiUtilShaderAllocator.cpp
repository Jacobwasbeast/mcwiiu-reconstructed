#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include "platform/wiiu/FFLiUtilShader.hpp"

FFLiUtilShaderAllocator::FFLiUtilShaderAllocator(FFLiBufferAllocator* allocator, unsigned int shaderCount, const void* shaderData, unsigned int shaderIndex)
{
    unsigned int programSize = FFLiUtilShader::GetProgramBufferSize(shaderCount, shaderData, shaderIndex);
    mProgramAllocator.Init((void*)allocator->Allocate(programSize, 256), programSize);
    unsigned int headerSize = FFLiUtilShader::GetHeaderBufferSize(shaderCount, shaderData, shaderIndex);
    mHeaderAllocator.Init((void*)allocator->Allocate(headerSize, 4), headerSize);
}

FFLiUtilShaderAllocator::FFLiUtilShaderAllocator(FFLiBufferAllocator* allocator, unsigned int shaderCount)
{
    unsigned int programSize = FFLiUtilShader::GetProgramBufferSizeForFetch(shaderCount);
    mProgramAllocator.Init((void*)allocator->Allocate(programSize, 256), programSize);
    unsigned int headerSize = FFLiUtilShader::GetHeaderBufferSizeForFetch(shaderCount);
    mHeaderAllocator.Init((void*)allocator->Allocate(headerSize, 4), headerSize);
}
