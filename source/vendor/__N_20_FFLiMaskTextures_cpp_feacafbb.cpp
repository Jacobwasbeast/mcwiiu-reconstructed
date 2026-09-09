#include "vendor/__N_20_FFLiMaskTextures_cpp_feacafbb.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiDatabaseDefault.hpp"
#include "platform/wiiu/FFLiCompressorImpl.hpp"

#include <string.h>

struct FFLiMaskTexturesTempObject
{
    unsigned char reserved[0x38];
    void* rawMasks[19];
};

extern uint32_t FFLiGetBufferRenderTexture(uint32_t width, uint32_t height, uint32_t format);
extern int FFLiiGetEyeRotateOffset(int value);

static int g_expressionParameters[19][5] =
{
    { -1, -1, 0, 0, 0 }
};

int32_t __N_20_FFLiMaskTextures_cpp_feacafbb::ExpressionFlagCount(uint32_t expressionMask)
{
    int32_t count = 0;
    uint32_t expression = 0;
    do
    {
        if ((expressionMask & (1u << (expression & 0x3f))) != 0)
            ++count;
        ++expression;
    }
    while (expression < 0x13);
    return count;
}

uint32_t __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
    uint32_t width, uint32_t height, bool compressed)
{
    if (!compressed)
        return FFLiGetBufferRenderTexture(width, height, 0x1a);
    return FFLiCompressorImpl<FFLiCompressorDescBC3>::GetTextureBufferSize(width, width, height);
}

uint32_t __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureFormat(bool compressed, bool alpha)
{
    if (!compressed)
        return alpha ? 0x41a : 0x1a;
    return alpha ? 0x433 : 0x33;
}

void __N_20_FFLiMaskTextures_cpp_feacafbb::FFLiRenderTextureAllocate(
    FFLiBufferAllocator* allocator)
{
    allocator->Allocate(0x9c, 4);
}

bool __N_20_FFLiMaskTextures_cpp_feacafbb::CanUseExpression(
    uint32_t expressionMask, FFLExpression expression)
{
    return (expressionMask & (1u << (static_cast<uint32_t>(expression) & 0x3f))) != 0;
}

void __N_20_FFLiMaskTextures_cpp_feacafbb::InitRawMask(
    FFLiMaskTexturesTempObject* object, uint32_t expressionMask, FFLiBufferAllocator* allocator)
{
    uint32_t expression = 0;
    do
    {
        if (CanUseExpression(expressionMask, static_cast<FFLExpression>(expression)))
            object->rawMasks[expression] = (void*)allocator->Allocate(0x3a8, 1);
        ++expression;
    }
    while (expression < 0x13);
}

int* __N_20_FFLiMaskTextures_cpp_feacafbb::GetCorrectParam(FFLExpression expression)
{
    return g_expressionParameters[static_cast<uint32_t>(expression)];
}

void __N_20_FFLiMaskTextures_cpp_feacafbb::SetupExpressionCharInfo(
    FFLiCharInfo* destination, const FFLiCharInfo*, FFLExpression expression)
{
    int* parameters = GetCorrectParam(expression);
    int value;
    memcpy(&value, parameters + 1, sizeof(value));
    if (value >= 0)
        memcpy(destination->data + 100, &value, sizeof(value));

    int rotation = parameters[2];
    memcpy(&value, parameters, sizeof(value));
    int current;
    memcpy(&current, destination->data + 0x20, sizeof(current));
    if (value >= 0 && value != current)
        rotation += FFLiiGetEyeRotateOffset(static_cast<int>(expression)) - FFLiiGetEyeRotateOffset(value);
    if (rotation != 0)
    {
        memcpy(&current, destination->data + 0x30, sizeof(current));
        current += rotation;
        if (current < 0) current = 0;
        else if (current > 7) current = 7;
        memcpy(destination->data + 0x30, &current, sizeof(current));
    }

    memcpy(&current, destination->data + 0x54, sizeof(current));
    current += parameters[4];
    memcpy(destination->data + 0x54, &current, sizeof(current));
    if (parameters[3] != 0)
    {
        memcpy(&current, destination->data + 0x4c, sizeof(current));
        current += parameters[3];
        if (current < 0) current = 0;
        else if (current > 0xb) current = 0xb;
        memcpy(destination->data + 0x4c, &current, sizeof(current));
    }
}
