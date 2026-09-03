#ifndef VENDOR___N_12_FFLIICON_CPP_65029AD9_HPP
#define VENDOR___N_12_FFLIICON_CPP_65029AD9_HPP

struct _GX2Texture;
struct FFLiCharModel;
struct FFLIconSetting;
struct FFLShaderCallback;
enum FFLExpression
{
    FFL_EXPRESSION_NONE = 0
};

class __N_12_FFLiIcon_cpp_65029ad9
{
public:
    static bool IsExpressionContained(const FFLiCharModel* charModel, FFLExpression expression);
    static unsigned int MakeIconImpl(_GX2Texture* texture, const FFLiCharModel* charModel,
                                     FFLExpression expression, const FFLIconSetting* setting,
                                     void* buffer, void* temporaryBuffer,
                                     const FFLShaderCallback* shaderCallback, bool flush);
    static double RadianToDegree(float radians);
    static bool IsTextureSizeValid(unsigned short size);
};

#endif
