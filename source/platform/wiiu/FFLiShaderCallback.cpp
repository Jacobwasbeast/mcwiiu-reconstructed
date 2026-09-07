#include "platform/wiiu/FFLiShaderCallback.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"

struct PlainShader;

extern void OSPanic(const char*, unsigned int, const char*, ...);
extern void GX2SetCullOnlyControl(unsigned int, int, int);
extern void GX2SetAttribBuffer(unsigned int, unsigned int, unsigned int, const void*);
extern void GX2DrawIndexedEx(unsigned int, unsigned int, unsigned int, const void*, unsigned int, unsigned int);
extern void SetModulate__FP11PlainShaderPC16FFLModulateParam(PlainShader*, const FFLModulateParam*);

struct _GX2ContextState;

typedef void (*FFLShaderCallbackDrawFunction)(const FFLShaderCallback*, const FFLDrawParam*);
typedef void (*FFLShaderCallbackMatrixFunction)(const FFLShaderCallback*, const _Mat44*);

struct FFLShaderCallback
{
    void* callbackData;
    const _GX2ContextState* contextState;
    FFLShaderCallbackDrawFunction draw;
    FFLShaderCallbackMatrixFunction setMatrix;
};

void FFLiTempSetContextState(const _GX2ContextState* contextState);

FFLiShaderCallback::FFLiShaderCallback()
    : m_callback(0)
{
}

FFLiShaderCallback::~FFLiShaderCallback()
{
}

bool FFLiShaderCallback::IsExist() const
{
    return m_callback != 0;
}

void FFLiShaderCallback::Set(const FFLShaderCallback* callback)
{
    m_callback = callback;
}

void FFLiShaderCallback::CallSetMatrix(const _Mat44& matrix) const
{
    if (IsExist())
    {
        m_callback->setMatrix(m_callback, &matrix);
    }
}

void FFLiShaderCallback::CallDraw(const FFLDrawParam& drawParam) const
{
    if (IsExist())
    {
        m_callback->draw(m_callback, &drawParam);
    }
}

void FFLiShaderCallback::CallSetContextState() const
{
    if (IsExist())
    {
        FFLiTempSetContextState(m_callback->contextState);
    }
}

void Draw__FPvPC12FFLDrawParam(void* shader, const FFLDrawParam* drawParam)
{
    if (shader == 0)
    {
        OSPanic(__FILE__, 0x137U, "Invalid Pointer: %x", 0);
    }
    if (drawParam == 0)
    {
        OSPanic(__FILE__, 0x138U, "Invalid Pointer: %x", 0);
    }

    GX2SetCullOnlyControl(0, 0, 0);
    SetModulate__FP11PlainShaderPC16FFLModulateParam((PlainShader*)shader, &drawParam->modulate);
    if (drawParam->flags != 0)
    {
        GX2SetAttribBuffer(0, drawParam->attributes.attributes[0].size,
                           drawParam->attributes.attributes[0].stride,
                           drawParam->attributes.attributes[0].buffer);
        GX2SetAttribBuffer(1, drawParam->attributes.attributes[1].size,
                           drawParam->attributes.attributes[1].stride,
                           drawParam->attributes.attributes[1].buffer);
        GX2DrawIndexedEx(drawParam->primitive.primitiveType,
                         drawParam->primitive.indexCount,
                         drawParam->primitive.vertexCount,
                         drawParam->primitive.indexBuffer, 0, 1);
    }
}
