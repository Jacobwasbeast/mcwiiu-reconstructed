#ifndef VENDOR___N_20_FFLIMASKTEXTURES_CPP_FEACAFBB_HPP
#define VENDOR___N_20_FFLIMASKTEXTURES_CPP_FEACAFBB_HPP

#include <stdint.h>

class FFLiBufferAllocator;
struct FFLiMaskTexturesTempObject;
struct FFLiCharInfo;

enum FFLExpression
{
    FFL_EXPRESSION_NONE = 0
};

class __N_20_FFLiMaskTextures_cpp_feacafbb
{
public:
    static int32_t ExpressionFlagCount(uint32_t expressionMask);
    static uint32_t GetTextureBufferSize(uint32_t width, uint32_t height, bool compressed);
    static uint32_t GetTextureFormat(bool compressed, bool alpha);
    static void FFLiRenderTextureAllocate(FFLiBufferAllocator* allocator);
    static bool CanUseExpression(uint32_t expressionMask, FFLExpression expression);
    static void InitRawMask(FFLiMaskTexturesTempObject* object, uint32_t expressionMask,
                            FFLiBufferAllocator* allocator);
    static int* GetCorrectParam(FFLExpression expression);
    static void SetupExpressionCharInfo(FFLiCharInfo* destination,
                                        const FFLiCharInfo* source, FFLExpression expression);
};

#endif
