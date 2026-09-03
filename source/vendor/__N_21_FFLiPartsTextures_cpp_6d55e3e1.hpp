#ifndef SOURCE_VENDOR___N_21_FFLIPARTSTEXTURES_CPP_6D55E3E1_HPP
#define SOURCE_VENDOR___N_21_FFLIPARTSTEXTURES_CPP_6D55E3E1_HPP

struct _GX2Texture;

class __N_21_FFLiPartsTextures_cpp_6d55e3e1
{
public:
    static void ExpressionToEyeUseFlag(bool* useFlags, unsigned int expression);
    static void ExpressionToMouthUseFlag(bool* useFlags, unsigned int expression);
    static void InvalidateTextures(_GX2Texture** textures, unsigned int count);
};

#endif
