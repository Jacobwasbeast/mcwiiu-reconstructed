#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

#include <cstring>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"

extern Renderer* gRenderer;

struct fuiCodeGenRectVertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

fuiRenderNodeCodeGenRect::fuiRenderNodeCodeGenRect(fuiRenderNode* parent, fuiRect* rect, const char* name,
                                                   unsigned int textureWidth, unsigned int textureHeight,
                                                   int textureId, bool customTexture, bool textureAlpha)
    : fuiRenderNode(parent, 0, 0, 0, eFuiObjectType_CodeGenRect, 0, 0, 0, false, 0),
      mTextureId(textureId), mCustomTexture(customTexture), mTextureAlpha(textureAlpha),
      mTextureWidth(textureWidth), mTextureHeight(textureHeight)
{
    m_bounds.xMin = rect->xMin;
    m_bounds.xMax = rect->xMax;
    m_bounds.yMin = rect->yMin;
    m_bounds.yMax = rect->yMax;
    std::strncpy(mName, name, sizeof(mName) - 1);
    mName[sizeof(mName) - 1] = 0;
}

fuiRenderNodeCodeGenRect::fuiRenderNodeCodeGenRect(fuiRenderNode* parent, fuiRect* rect, const char* name)
    : fuiRenderNode(parent, 0, 0, 0, eFuiObjectType_CodeGenRect, 0, 0, 0, false, 0),
      mTextureId(-1), mCustomTexture(0), mTextureAlpha(0), mTextureWidth(0), mTextureHeight(0)
{
    m_bounds.xMin = rect->xMin;
    m_bounds.xMax = rect->xMax;
    m_bounds.yMin = rect->yMin;
    m_bounds.yMax = rect->yMax;
    std::strncpy(mName, name, sizeof(mName) - 1);
    mName[sizeof(mName) - 1] = 0;
}

fuiRenderNodeCodeGenRect::~fuiRenderNodeCodeGenRect()
{
}

void fuiRenderNodeCodeGenRect::gather(bool recurse)
{
    if (recurse)
    {
        progogateBounds();
    }
}

void fuiRenderNodeCodeGenRect::preRender()
{
    generateGlobalMatrix();
}

void fuiRenderNodeCodeGenRect::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
    {
        return;
    }
    pushMatrix();
    pushColor();
    pushFilterFlags();
    if (bounds != NULL)
    {
        bounds->xMin = m_bounds.xMin;
        bounds->xMax = m_bounds.xMax;
        bounds->yMin = m_bounds.yMin;
        bounds->yMax = m_bounds.yMax;
    }
    fuiCodeGenRectVertex vertices[4];
    vertices[0].x = m_bounds.xMin; vertices[0].y = m_bounds.yMin; vertices[0].z = 0.0f; vertices[0].u = 0.0f; vertices[0].v = 0.0f;
    vertices[1].x = m_bounds.xMax; vertices[1].y = m_bounds.yMin; vertices[1].z = 0.0f; vertices[1].u = 1.0f; vertices[1].v = 0.0f;
    vertices[2].x = m_bounds.xMax; vertices[2].y = m_bounds.yMax; vertices[2].z = 0.0f; vertices[2].u = 1.0f; vertices[2].v = 1.0f;
    vertices[3].x = m_bounds.xMin; vertices[3].y = m_bounds.yMax; vertices[3].z = 0.0f; vertices[3].u = 0.0f; vertices[3].v = 1.0f;
    if (gRenderer != NULL)
    {
        gRenderer->TextureBind(mTextureId);
        gRenderer->StateSetTextureScaleUI(1.0f, 1.0f, 0.0f, 0.0f);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_TriangleFan, 4, vertices,
                                C4JRender::eVertexType_UI, C4JRender::ePixelShaderType_UI, NULL);
    }
    popFilterFlags();
    popColor();
    popMatrix();
}

void fuiRenderNodeCodeGenRect::setWidth(float width)
{
    m_bounds.xMax = m_bounds.xMin + width;
}

void fuiRenderNodeCodeGenRect::setHeight(float height)
{
    m_bounds.yMax = m_bounds.yMin + height;
}

void fuiRenderNodeCodeGenRect::setTexture(unsigned int textureWidth, unsigned int textureHeight,
                                          int textureId, bool textureAlpha)
{
    if (mTextureId >= 0)
    {
        if (!mCustomTexture)
        {
            gRenderer->TextureFree(mTextureId);
        }
        else if (!mTextureAlpha)
        {
            gFui->releaseCustomTexture(mTextureId);
        }
    }
    mTextureWidth = textureWidth;
    mTextureHeight = textureHeight;
    mTextureId = textureId;
    mTextureAlpha = textureAlpha;
}
