#include "platform/wiiu/FFLiCompressDrawer.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include <cstring>

extern "C"
{
void GX2Invalidate(unsigned int invalidateType, void* address, unsigned int size);
void GX2SetAttribBuffer(unsigned int index, unsigned int size, unsigned int stride, const void* buffer);
void GX2DrawEx(unsigned int mode, unsigned int count, unsigned int offset, unsigned int numInstances);
}

FFLiCompressDrawer::FFLiCompressDrawer()
{
    mVertexBuffer = 0;
    mTexCoordBuffer = 0;
}

FFLiCompressDrawer::~FFLiCompressDrawer()
{
}

void FFLiCompressDrawer::Setup(FFLiBufferAllocator* allocator)
{
    SetupCPU(allocator);
    SetupGPU();
}

void FFLiCompressDrawer::SetupCPU(FFLiBufferAllocator* allocator)
{
    mVertexBuffer = allocator->Allocate(0x30U, 0x40U);
    mTexCoordBuffer = allocator->Allocate(0x20U, 0x40U);
    const unsigned int vertexData[12] = {
        0xbf800000U, 0x3f800000U, 0x00000000U, 0xbf800000U,
        0xbf800000U, 0x00000000U, 0x3f800000U, 0x3f800000U,
        0x00000000U, 0x3f800000U, 0x3f800000U, 0x00000000U
    };
    const unsigned int texCoordData[8] = {
        0x00000000U, 0x00000000U, 0x00000000U, 0x3f800000U,
        0x3f800000U, 0x00000000U, 0x00000000U, 0x3f800000U
    };
    std::memcpy((void*)mVertexBuffer, vertexData, 0x30U);
    std::memcpy((void*)mTexCoordBuffer, texCoordData, 0x20U);
}

void FFLiCompressDrawer::SetupGPU()
{
    GX2Invalidate(0x41U, (void*)mVertexBuffer, 0x30U);
    GX2Invalidate(0x41U, (void*)mTexCoordBuffer, 0x20U);
}

void FFLiCompressDrawer::SetAttributeBuffer(unsigned int index, unsigned int bufferIndex)
{
    GX2SetAttribBuffer(index, 0x30U, 0x0cU, (const void*)mVertexBuffer);
    GX2SetAttribBuffer(bufferIndex, 0x20U, 8U, (const void*)mTexCoordBuffer);
}

void FFLiCompressDrawer::Draw()
{
    GX2DrawEx(6U, 4U, 0U, 1U);
}

unsigned int FFLiCompressDrawer::GetBufferSize()
{
    return 0xd0U;
}
