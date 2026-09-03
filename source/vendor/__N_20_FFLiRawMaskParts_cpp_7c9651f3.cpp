#include "vendor/__N_20_FFLiRawMaskParts_cpp_7c9651f3.hpp"

#include <cstring>

unsigned int FFLiBugCanSwapSize(unsigned int size);
unsigned int FFLiBugCanVgtFixedIndexSize(void);
void* FFLiBugVgtFixedIndexPtr(void* value);
void FFLiInvalidate(unsigned int type, void* value, unsigned int size);
void ASM_MTXScale(double x, double y, double z, _Mat34* matrix);
void ASM_MTXRotRad(double angle, _Mat34* matrix, unsigned int axis);
void ASM_MTXConcat(const _Mat34* left, const _Mat34* right, _Mat34* result);
void ASM_MTXTrans(double x, double y, double z, _Mat34* matrix);
void ASM_MTX44MultVec(const _Mat44* matrix, FFLVec4* source, FFLVec4* result);

uint32_t __N_20_FFLiRawMaskParts_cpp_7c9651f3::Allocate(FFLiBufferAllocator* allocator, uint32_t size, uint32_t alignment)
{
    return allocator->Allocate(size, alignment);
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::EndianSwap(void* data, uint32_t size)
{
    (void)data;
    (void)size;
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::CalcMVMatrix(_Mat34* matrix, const FFLiRawMaskPartsDesc* description)
{
    _Mat34 temporary;
    ASM_MTXScale(description->scaleX, description->scaleY, 1.0, matrix);
    ASM_MTXRotRad(description->rotation * 0.01745329238474369, &temporary, 122U);
    ASM_MTXConcat(&temporary, matrix, matrix);
    ASM_MTXScale(0.8896146416664124, 0.927667498588562, 1.0, &temporary);
    ASM_MTXConcat(&temporary, matrix, matrix);
    ASM_MTXTrans(description->x, description->y, 0.0, &temporary);
    ASM_MTXConcat(&temporary, matrix, matrix);
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::InitPrimitive(FFLPrimitiveParam* primitive, FFLiBufferAllocator* allocator)
{
    primitive->primitiveType = 6U;
    primitive->vertexCount = 4U;
    primitive->indexCount = 4U;
    FFLiBugCanSwapSize(8U);
    uint32_t size = FFLiBugCanVgtFixedIndexSize();
    void* indexBuffer = (void*)Allocate(allocator, size, 32U);
    indexBuffer = FFLiBugVgtFixedIndexPtr(indexBuffer);
    primitive->indexBuffer = indexBuffer;
    static const uint8_t indices[8] = { 0, 2, 0, 3, 0xbf, 0, 0xbf, 0x80 };
    std::memcpy(indexBuffer, indices, sizeof(indices));
    EndianSwap(indexBuffer, sizeof(indices));
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::CalcAttribute(FFLVec4* positions, FFLVec2* texCoords, FFLiOriginPosition origin, const _Mat44* matrix)
{
    static const float positionX[4] = { 1.0f, 1.0f, 0.0f, 0.0f };
    static const float positionY[4] = { 1.0f, 0.0f, 0.0f, -0.5f };
    static const float texY[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
    float xOffset = 0.0f;
    float uRight = 0.0f;
    float uLeft = 1.0f;
    float yOffset = 0.0f;
    if (origin == FFLiOriginPositionTopLeft)
    {
        xOffset = -0.5f;
    }
    else if (origin == FFLiOriginPositionTopRight)
    {
        yOffset = 1.0f;
    }
    else if (origin == FFLiOriginPositionBottomLeft)
    {
        xOffset = -1.0f;
    }
    for (unsigned int index = 0; index < 4; ++index)
    {
        positions[index].x = positionX[index] + xOffset;
        positions[index].y = positionY[index];
        positions[index].z = 0.0f;
        positions[index].w = 0.0f;
        texCoords[index].y = texY[index];
    }
    texCoords[0].x = uLeft;
    texCoords[1].x = uLeft;
    texCoords[2].x = uRight;
    texCoords[3].x = uRight;
    for (unsigned int index = 0; index < 4; ++index)
        ASM_MTX44MultVec(matrix, &positions[index], &positions[index]);
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::InitAttributes(FFLAttributeBufferParam* attributes, FFLiOriginPosition origin, FFLiBufferAllocator* allocator, const _Mat44* matrix)
{
    static const uint8_t defaults[60] = { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x10 };
    std::memcpy(attributes, defaults, sizeof(defaults));
    uint32_t positionSize = FFLiBugCanSwapSize(64U);
    attributes->buffers[0].buffer = (void*)Allocate(allocator, positionSize, 64U);
    uint32_t texCoordSize = FFLiBugCanSwapSize(32U);
    attributes->buffers[1].buffer = (void*)Allocate(allocator, texCoordSize, 64U);
    CalcAttribute((FFLVec4*)attributes->buffers[0].buffer, (FFLVec2*)attributes->buffers[1].buffer, origin, matrix);
    EndianSwap(attributes->buffers[0].buffer, 64U);
    EndianSwap(attributes->buffers[1].buffer, 32U);
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::InvalidatePrimitive(FFLPrimitiveParam* primitive)
{
    FFLiInvalidate(65U, primitive->indexBuffer, primitive->vertexCount << 1);
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::InvalidateAttributes(FFLAttributeBufferParam* attributes)
{
    for (unsigned int index = 0; index < 5; ++index)
    {
        if (attributes->buffers[index].buffer != 0)
            FFLiInvalidate(65U, attributes->buffers[index].buffer, attributes->buffers[index].size);
    }
}

void __N_20_FFLiRawMaskParts_cpp_7c9651f3::InitAttributesForFill(FFLAttributeBufferParam* attributes, FFLiBufferAllocator* allocator)
{
    static const uint8_t defaults[60] = { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x10 };
    static const uint8_t fill[64] = { 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0xbf };
    std::memcpy(attributes, defaults, sizeof(defaults));
    uint32_t size = FFLiBugCanSwapSize(64U);
    attributes->buffers[0].buffer = (void*)Allocate(allocator, size, 64U);
    std::memcpy(attributes->buffers[0].buffer, fill, sizeof(fill));
    EndianSwap(attributes->buffers[0].buffer, sizeof(fill));
}
