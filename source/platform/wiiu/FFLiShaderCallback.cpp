#include "platform/wiiu/FFLiShaderCallback.hpp"

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
