#ifndef VENDOR___N_20_FFLIMASKTEXTURES_CPP_FEACAFBB_HPP
#define VENDOR___N_20_FFLIMASKTEXTURES_CPP_FEACAFBB_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiTypes.hpp"
#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"

class FFLiBufferAllocator;
struct FFLiMaskTexturesTempObject;
struct FFLiCharInfo;
struct FFLiRenderTexture;

class __N_20_FFLiMaskTextures_cpp_feacafbb
{
public:
    static int32_t ExpressionFlagCount(uint32_t expressionMask);
    static uint32_t GetTextureBufferSize(uint32_t resolution, uint32_t mipCount, bool compressed);
    static uint32_t GetTextureFormat(bool compressed, bool alpha);
    static FFLiRenderTexture* FFLiRenderTextureAllocate(FFLiBufferAllocator* allocator);
    static bool CanUseExpression(uint32_t expressionMask, FFLExpression expression);
    static void InitRawMask(FFLiMaskTexturesTempObject* object, uint32_t expressionMask,
                            FFLiBufferAllocator* allocator);
    static int* GetCorrectParam(FFLExpression expression);
    static void SetupExpressionCharInfo(FFLiCharInfo* destination,
                                        const FFLiCharInfo* source, FFLExpression expression);
};

#endif
