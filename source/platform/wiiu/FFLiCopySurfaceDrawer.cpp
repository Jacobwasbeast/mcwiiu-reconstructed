#include "platform/wiiu/FFLiCopySurfaceDrawer.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include <cstring>

extern "C"
{
void GX2Invalidate(unsigned int invalidateType, void* address, unsigned int size);
void GX2SetAttribBuffer(unsigned int index, unsigned int size, unsigned int stride, const void* buffer);
void GX2DrawEx(unsigned int mode, unsigned int count, unsigned int offset, unsigned int numInstances);
}

FFLiCopySurfaceDrawer::FFLiCopySurfaceDrawer()
{
    mPositionBuffer = 0;
    mTexCoordBuffer = 0;
}

FFLiCopySurfaceDrawer::~FFLiCopySurfaceDrawer()
{
}

void FFLiCopySurfaceDrawer::SetupCPU(FFLiBufferAllocator* allocator)
{
    mPositionBuffer = allocator->Allocate(0x20U, 0x40U);
    mTexCoordBuffer = allocator->Allocate(0x20U, 0x40U);
    const unsigned int positionData[8] = {
        0xbf800000U, 0x3f800000U, 0xbf800000U, 0xbf800000U,
        0x3f800000U, 0x3f800000U, 0x3f800000U, 0xbf800000U
    };
    const unsigned int texCoordData[8] = {
        0x00000000U, 0x00000000U, 0x00000000U, 0x3f800000U,
        0x3f800000U, 0x00000000U, 0x00000000U, 0x3f800000U
    };
    std::memcpy((void*)mPositionBuffer, positionData, 0x20U);
    std::memcpy((void*)mTexCoordBuffer, texCoordData, 0x20U);
}

void FFLiCopySurfaceDrawer::SetupGPU()
{
    GX2Invalidate(0x41U, (void*)mPositionBuffer, 0x20U);
    GX2Invalidate(0x41U, (void*)mTexCoordBuffer, 0x20U);
}

void FFLiCopySurfaceDrawer::SetAttributeBuffer(unsigned int index, unsigned int bufferIndex)
{
    GX2SetAttribBuffer(index, 0x20U, 8U, (const void*)mPositionBuffer);
    GX2SetAttribBuffer(bufferIndex, 0x20U, 8U, (const void*)mTexCoordBuffer);
}

void FFLiCopySurfaceDrawer::Draw()
{
    GX2DrawEx(6U, 4U, 0U, 1U);
}

unsigned int FFLiCopySurfaceDrawer::GetBufferSize()
{
    return 0xc0U;
}
