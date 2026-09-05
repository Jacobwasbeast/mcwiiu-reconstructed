#ifndef SOURCE_VENDOR_N_21_FFLIRENDERTEXTURE_CPP_7BE96556_HPP
#define SOURCE_VENDOR_N_21_FFLIRENDERTEXTURE_CPP_7BE96556_HPP

struct FFLColor;
struct _GX2Surface;

class __N_21_FFLiRenderTexture_cpp_7be96556
{
public:
    static void FillColorRGBA8(void* destination, unsigned int size, const FFLColor& color);
    static void DumpSurface(const _GX2Surface* surface);
};

#endif
