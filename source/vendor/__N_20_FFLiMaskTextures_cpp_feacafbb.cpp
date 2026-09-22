#include "vendor/__N_20_FFLiMaskTextures_cpp_feacafbb.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiDatabaseDefault.hpp"
#include "platform/wiiu/FFLiCompressorImpl.hpp"
#include "platform/wiiu/FFL.hpp"
#include <gx2/enum.h>

#include <string.h>
#include <stdint.h>

namespace
{
const uint32_t kExpressionCount = 19;
const uint32_t kRawMaskAllocationSize = 936;
const uint32_t kRenderTextureAllocationSize = 156;
const uint32_t kExpressionBitMask = 0x3f;
const uint32_t kTextureFormatUncompressedAlpha = 0x41a;
const uint32_t kTextureFormatCompressed = 0x33;
const uint32_t kTextureFormatCompressedAlpha = 0x433;
const int kMinimumExpressionValue = 0;
const int kMaximumEyeRotation = 7;
const int kMaximumExpressionAdjustment = 11;

const uint32_t kEyeTypeField = 0x20;
const uint32_t kEyeRotationField = 0x30;
const uint32_t kExpressionField = 0x4c;
const uint32_t kMouthField = 0x54;
const uint32_t kExpressionParameterField = 100;

int ReadCharInfoField(const FFLiCharInfo* info, uint32_t field)
{
    int value;
    memcpy(&value, info->data + field, sizeof(value));
    return value;
}

void WriteCharInfoField(FFLiCharInfo* info, uint32_t field, int value)
{
    memcpy(info->data + field, &value, sizeof(value));
}
}

static int g_expressionParameters[19][5] =
{
    { -1, -1, 0, 0, 0 },
    { 60, -1, 0, 0, 0 },
    { -1, 10, 2, 2, 0 },
    { -1, 12, -2, -2, 0 },
    { 61, -1, 0, 0, -2 },
    { 26, -1, 0, 0, 0 },
    { -1, 36, 0, 0, 0 },
    { 60, 36, 0, 0, 0 },
    { -1, 36, 2, 2, 0 },
    { -1, 36, -2, -2, 0 },
    { 61, 36, 0, 0, -2 },
    { 26, 36, 0, 0, 0 },
    { 47, -1, 0, 0, 0 },
    { 47, -1, 0, 0, 0 },
    { 47, 36, 0, 0, 0 },
    { 47, 36, 0, 0, 0 },
    { 47, -1, 0, 0, 0 },
    { 47, -1, 0, 0, 0 },
    { 47, 12, 0, 0, 0 }
};

int32_t __N_20_FFLiMaskTextures_cpp_feacafbb::ExpressionFlagCount(uint32_t expressionMask)
{
    int32_t count = 0;
    uint32_t expression = 0;
    do
    {
        if ((expressionMask & (1u << (expression & kExpressionBitMask))) != 0)
            ++count;
        ++expression;
    }
    while (expression < kExpressionCount);
    return count;
}

uint32_t __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
    uint32_t resolution, uint32_t mipCount, bool compressed)
{
    if (!compressed)
        return static_cast<uint32_t>(
            FFLiGetBufferRenderTexture(resolution, resolution,
                                       GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, mipCount));
    return FFLiCompressorImpl<FFLiCompressorDescBC3>::GetTextureBufferSize(
        resolution, resolution, mipCount);
}

uint32_t __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureFormat(bool compressed, bool alpha)
{
    if (!compressed)
        return alpha ? kTextureFormatUncompressedAlpha : GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
        return alpha ? kTextureFormatCompressedAlpha : kTextureFormatCompressed;
}

FFLiRenderTexture* __N_20_FFLiMaskTextures_cpp_feacafbb::FFLiRenderTextureAllocate(
    FFLiBufferAllocator* allocator)
{
    return reinterpret_cast<FFLiRenderTexture*>(
        static_cast<uintptr_t>(allocator->Allocate(kRenderTextureAllocationSize, 4)));
}

bool __N_20_FFLiMaskTextures_cpp_feacafbb::CanUseExpression(
    uint32_t expressionMask, FFLExpression expression)
{
    return (expressionMask & (1u << (static_cast<uint32_t>(expression) & kExpressionBitMask))) != 0;
}

void __N_20_FFLiMaskTextures_cpp_feacafbb::InitRawMask(
    FFLiMaskTexturesTempObject* object, uint32_t expressionMask, FFLiBufferAllocator* allocator)
{
    uint32_t expression = 0;
    do
    {
        if (CanUseExpression(expressionMask, static_cast<FFLExpression>(expression)))
            object->rawMasks[expression] = reinterpret_cast<FFLiRawMaskDrawParam*>(
                static_cast<uintptr_t>(allocator->Allocate(kRawMaskAllocationSize, 1)));
        ++expression;
    }
    while (expression < kExpressionCount);
}

const int* __N_20_FFLiMaskTextures_cpp_feacafbb::GetCorrectParam(FFLExpression expression)
{
    return g_expressionParameters[static_cast<uint32_t>(expression)];
}

void __N_20_FFLiMaskTextures_cpp_feacafbb::SetupExpressionCharInfo(
    FFLiCharInfo* destination, const FFLiCharInfo* source, FFLExpression expression)
{
    (void)source;
    const int* parameters = GetCorrectParam(expression);
    if (parameters[1] >= kMinimumExpressionValue)
        WriteCharInfoField(destination, kExpressionParameterField, parameters[1]);

    int rotation = parameters[2];
    const int eyeType = parameters[0];
    const int currentEyeType = ReadCharInfoField(destination, kEyeTypeField);
    if (eyeType >= kMinimumExpressionValue && eyeType != currentEyeType)
        rotation += FFLiiGetEyeRotateOffset(static_cast<int>(expression)) - FFLiiGetEyeRotateOffset(eyeType);
    if (rotation != 0)
    {
        int current = ReadCharInfoField(destination, kEyeRotationField);
        current += rotation;
        if (current < kMinimumExpressionValue) current = kMinimumExpressionValue;
        else if (current > kMaximumEyeRotation) current = kMaximumEyeRotation;
        WriteCharInfoField(destination, kEyeRotationField, current);
    }

    int current = ReadCharInfoField(destination, kMouthField);
    current += parameters[4];
    WriteCharInfoField(destination, kMouthField, current);
    if (parameters[3] != 0)
    {
        current = ReadCharInfoField(destination, kExpressionField);
        current += parameters[3];
        if (current < kMinimumExpressionValue) current = kMinimumExpressionValue;
        else if (current > kMaximumExpressionAdjustment) current = kMaximumExpressionAdjustment;
        WriteCharInfoField(destination, kExpressionField, current);
    }
}
