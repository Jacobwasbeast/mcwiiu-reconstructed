#include "vendor/__N_13_FFLiColor_cpp_87b9af10.hpp"

#include <string.h>

static FFLiColorContainer g_colorContainer;
static FFLiColorContainer g_srgbFetchColorContainer;
static unsigned char g_colorInitialized;
static unsigned char g_srgbFetchInitialized;
static int g_colorContainerIndex;

const float *__N_13_FFLiColor_cpp_87b9af10::GetFacelineColor(const FFLiColorContainer &container, int index)
{
    return container.color + index * 4;
}

void __N_13_FFLiColor_cpp_87b9af10::SetupColorContainer(FFLiColorContainer &container)
{
    static const float faceline[24] = { 1.0f, 1.0f, 0.824f, 0.677f, 1.0f, 1.0f, 0.824f, 0.677f, 1.0f, 1.0f, 0.824f, 0.677f, 1.0f, 1.0f, 0.824f, 0.677f, 1.0f, 1.0f, 0.824f, 0.677f, 1.0f, 1.0f, 0.824f, 0.677f };
    static const float beard[32] = { 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f, 1.0f, 0.118f, 0.102f, 0.094f };
    static const float eye[24] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    static const float mouthR[20] = { 1.0f, 0.847f, 0.322f, 0.124f, 1.0f, 0.847f, 0.322f, 0.124f, 1.0f, 0.847f, 0.322f, 0.124f, 1.0f, 0.847f, 0.322f, 0.124f, 1.0f, 0.847f, 0.322f, 0.124f };
    static const float mouthG[20] = { 1.0f, 0.51f, 0.188f, 0.094f, 1.0f, 0.51f, 0.188f, 0.094f, 1.0f, 0.51f, 0.188f, 0.094f, 1.0f, 0.51f, 0.188f, 0.094f, 1.0f, 0.51f, 0.188f, 0.094f };
    static const float mouthB[20] = { 1.0f, 0.824f, 0.118f, 0.078f, 1.0f, 0.824f, 0.118f, 0.078f, 1.0f, 0.824f, 0.118f, 0.078f, 1.0f, 0.824f, 0.118f, 0.078f, 1.0f, 0.824f, 0.118f, 0.078f };
    memset(&container, 0, sizeof(container));
    memcpy(container.color, faceline, sizeof(faceline));
    memcpy(container.color + 24, beard, sizeof(beard));
    memcpy(container.color + 56, eye, sizeof(eye));
    memcpy(container.color + 72, eye, sizeof(eye));
    memcpy(container.color + 88, mouthR, sizeof(mouthR));
    memcpy(container.color + 108, mouthG, sizeof(mouthG));
    memcpy(container.color + 128, mouthB, sizeof(mouthB));
    container.color[160] = 1.0f;
    container.color[161] = 1.0f;
    container.color[162] = 1.0f;
    container.color[163] = 1.0f;
    container.color[164] = 1.0f;
    container.color[165] = 1.0f;
    container.color[166] = 1.0f;
    container.color[167] = 1.0f;
    container.color[168] = 1.0f;
    container.color[169] = 1.0f;
    container.color[170] = 1.0f;
    container.color[171] = 1.0f;
    container.color[172] = 1.0f;
    container.color[173] = 1.0f;
    container.color[174] = 1.0f;
    container.color[175] = 1.0f;
    container.color[176] = 1.0f;
    container.color[177] = 1.0f;
    container.color[178] = 1.0f;
    container.color[179] = 1.0f;
    container.color[180] = 1.0f;
    container.color[181] = 1.0f;
    container.color[182] = 1.0f;
    container.color[183] = 1.0f;
    container.color[184] = 1.0f;
    container.color[185] = 1.0f;
    container.color[186] = 1.0f;
    container.color[187] = 1.0f;
    container.color[188] = 1.0f;
    container.color[189] = 1.0f;
    container.color[190] = 1.0f;
    container.color[191] = 1.0f;
    container.color[192] = 1.0f;
    container.color[193] = 1.0f;
    container.color[194] = 1.0f;
    container.color[195] = 1.0f;
    container.color[196] = 1.0f;
    container.color[197] = 1.0f;
    container.color[198] = 1.0f;
    container.color[199] = 1.0f;
    container.color[200] = 1.0f;
    container.color[201] = 1.0f;
    container.color[202] = 1.0f;
    container.color[203] = 1.0f;
    container.color[204] = 1.0f;
    container.color[205] = 1.0f;
    container.color[206] = 1.0f;
    container.color[207] = 1.0f;
    container.color[208] = 1.0f;
    container.color[209] = 1.0f;
    container.color[210] = 1.0f;
    container.color[211] = 1.0f;
    container.color[212] = 1.0f;
    container.color[213] = 1.0f;
    container.color[214] = 1.0f;
    container.color[215] = 1.0f;
    container.color[216] = 1.0f;
    container.color[217] = 1.0f;
    container.color[218] = 1.0f;
    container.color[219] = 1.0f;
}

double __N_13_FFLiColor_cpp_87b9af10::SrgbFetchValue(float value)
{
    const unsigned char table[256] = { 0 };
    unsigned int index = (unsigned int)(value * 255.0f) & 255u;
    return ((double)table[index] - 0.0) / 255.0;
}

void __N_13_FFLiColor_cpp_87b9af10::DegammaColorContainer(FFLiColorContainer &container)
{
    unsigned int i;
    for (i = 0; i < 55; ++i)
    {
        container.color[i * 4] = (float)SrgbFetchValue(container.color[i * 4]);
        container.color[i * 4 + 1] = (float)SrgbFetchValue(container.color[i * 4 + 1]);
        container.color[i * 4 + 2] = (float)SrgbFetchValue(container.color[i * 4 + 2]);
    }
}

void __N_13_FFLiColor_cpp_87b9af10::InitializeColorContainerIfUninitialized()
{
    if (g_colorInitialized == 0)
    {
        SetupColorContainer(g_colorContainer);
        SetupColorContainer(g_srgbFetchColorContainer);
        DegammaColorContainer(g_srgbFetchColorContainer);
        g_colorInitialized = 1;
    }
}

FFLiColorContainer *__N_13_FFLiColor_cpp_87b9af10::GetColorContainer()
{
    InitializeColorContainerIfUninitialized();
    return &g_colorContainer + g_colorContainerIndex;
}

FFLiColorContainer *__N_13_FFLiColor_cpp_87b9af10::GetSrgbFetchColorContainer()
{
    InitializeColorContainerIfUninitialized();
    if (g_srgbFetchInitialized == 0)
        g_srgbFetchInitialized = 1;
    return &g_srgbFetchColorContainer;
}

const float *__N_13_FFLiColor_cpp_87b9af10::GetBeardColor(const FFLiColorContainer &container, int index) { return container.color + 24 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetEyebrowColor(const FFLiColorContainer &container, int index) { return container.color + 24 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetMustacheColor(const FFLiColorContainer &container, int index) { return container.color + 24 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetHairColor(const FFLiColorContainer &container, int index) { return container.color + 24 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetEyeColorR(const FFLiColorContainer &container, int) { return container.color + 64; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetEyeColorG(const FFLiColorContainer &container, int) { return container.color + 68; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetEyeColorB(const FFLiColorContainer &container, int index) { return container.color + 72 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetGlassColor(const FFLiColorContainer &container, int index) { return container.color + 96 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetMouthColorR(const FFLiColorContainer &container, int index) { return container.color + 120 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetMouthColorG(const FFLiColorContainer &container, int index) { return container.color + 140 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetMouthColorB(const FFLiColorContainer &container, int) { return container.color + 160; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetMoleColor(const FFLiColorContainer &container) { return container.color + 164; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetFavoriteColor(const FFLiColorContainer &container, int index) { return container.color + 168 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetFaceLine(const FFLiColorContainer &container) { return container.color + 216; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetCapColor(const FFLiColorContainer &container, int index) { return container.color + 168 + index * 4; }
const float *__N_13_FFLiColor_cpp_87b9af10::GetNoselineColor(const FFLiColorContainer &container) { return container.color + 216; }

int __N_13_FFLiColor_cpp_87b9af10::GetColorNum(FFLiColorType type)
{
    switch (type)
    {
    case FFLI_COLOR_FACELINE: return 6;
    case FFLI_COLOR_BEARD: return 8;
    case FFLI_COLOR_EYEBROW: return 8;
    case FFLI_COLOR_MUSTACHE: return 8;
    case FFLI_COLOR_HAIR: return 8;
    case FFLI_COLOR_EYE: return 6;
    case FFLI_COLOR_GLASS: return 6;
    case FFLI_COLOR_MOUTH: return 5;
    case FFLI_COLOR_MOLE: return 1;
    case FFLI_COLOR_FAVORITE: return 12;
    case FFLI_COLOR_CAP: return 12;
    case FFLI_COLOR_FACE_LINE: return 1;
    default: return 0;
    }
}

const float *__N_13_FFLiColor_cpp_87b9af10::GetColor(FFLiColorType type, unsigned int index)
{
    const FFLiColorContainer &container = *GetColorContainer();
    switch (type)
    {
    case FFLI_COLOR_FACELINE: return GetFacelineColor(container, (int)index);
    case FFLI_COLOR_BEARD: return GetBeardColor(container, (int)index);
    case FFLI_COLOR_EYEBROW: return GetEyebrowColor(container, (int)index);
    case FFLI_COLOR_MUSTACHE: return GetMustacheColor(container, (int)index);
    case FFLI_COLOR_HAIR: return GetHairColor(container, (int)index);
    case FFLI_COLOR_EYE: return GetEyeColorB(container, (int)index);
    case FFLI_COLOR_GLASS: return GetGlassColor(container, (int)index);
    case FFLI_COLOR_MOUTH: return GetMouthColorR(container, (int)index);
    case FFLI_COLOR_MOLE: return GetMoleColor(container);
    case FFLI_COLOR_FAVORITE: return GetFavoriteColor(container, (int)index);
    case FFLI_COLOR_CAP: return GetCapColor(container, (int)index);
    case FFLI_COLOR_FACE_LINE: return GetFaceLine(container);
    default: return 0;
    }
}
