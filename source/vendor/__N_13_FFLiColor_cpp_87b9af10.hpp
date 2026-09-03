#ifndef SOURCE_VENDOR___N_13_FFLICOLOR_CPP_87B9AF10_HPP
#define SOURCE_VENDOR___N_13_FFLICOLOR_CPP_87B9AF10_HPP

#include <stdint.h>

struct FFLiColorContainer
{
    float color[220];
};

enum FFLiColorType
{
    FFLI_COLOR_FACELINE = 0,
    FFLI_COLOR_BEARD = 1,
    FFLI_COLOR_EYEBROW = 2,
    FFLI_COLOR_MUSTACHE = 3,
    FFLI_COLOR_HAIR = 4,
    FFLI_COLOR_EYE = 5,
    FFLI_COLOR_GLASS = 6,
    FFLI_COLOR_MOUTH = 7,
    FFLI_COLOR_MOLE = 8,
    FFLI_COLOR_FAVORITE = 9,
    FFLI_COLOR_CAP = 10,
    FFLI_COLOR_FACE_LINE = 11
};

class __N_13_FFLiColor_cpp_87b9af10
{
public:
    static const float *GetFacelineColor(const FFLiColorContainer &, int);
    static void SetupColorContainer(FFLiColorContainer &);
    static double SrgbFetchValue(float);
    static void DegammaColorContainer(FFLiColorContainer &);
    static void InitializeColorContainerIfUninitialized();
    static FFLiColorContainer *GetColorContainer();
    static FFLiColorContainer *GetSrgbFetchColorContainer();
    static const float *GetBeardColor(const FFLiColorContainer &, int);
    static const float *GetEyebrowColor(const FFLiColorContainer &, int);
    static const float *GetMustacheColor(const FFLiColorContainer &, int);
    static const float *GetHairColor(const FFLiColorContainer &, int);
    static const float *GetEyeColorR(const FFLiColorContainer &, int);
    static const float *GetEyeColorG(const FFLiColorContainer &, int);
    static const float *GetEyeColorB(const FFLiColorContainer &, int);
    static const float *GetGlassColor(const FFLiColorContainer &, int);
    static const float *GetMouthColorR(const FFLiColorContainer &, int);
    static const float *GetMouthColorG(const FFLiColorContainer &, int);
    static const float *GetMouthColorB(const FFLiColorContainer &, int);
    static const float *GetMoleColor(const FFLiColorContainer &);
    static const float *GetFavoriteColor(const FFLiColorContainer &, int);
    static const float *GetFaceLine(const FFLiColorContainer &);
    static const float *GetCapColor(const FFLiColorContainer &, int);
    static const float *GetNoselineColor(const FFLiColorContainer &);
    static int GetColorNum(FFLiColorType);
    static const float *GetColor(FFLiColorType, unsigned int);
};

#endif
