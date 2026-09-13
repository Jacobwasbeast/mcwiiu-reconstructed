#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

#include <cstring>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"

extern Renderer* gRenderer;

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
    std::strncpy(m_name, name, sizeof(m_name) - 1);
    m_name[sizeof(m_name) - 1] = 0;
}

fuiRenderNodeCodeGenRect::fuiRenderNodeCodeGenRect(fuiRenderNode* parent, fuiRect* rect, const char* name)
    : fuiRenderNode(parent, 0, 0, 0, eFuiObjectType_CodeGenRect, 0, 0, 0, false, 0),
      mTextureId(-1), mCustomTexture(0), mTextureAlpha(0), mTextureWidth(0), mTextureHeight(0)
{
    m_bounds.xMin = rect->xMin;
    m_bounds.xMax = rect->xMax;
    m_bounds.yMin = rect->yMin;
    m_bounds.yMax = rect->yMax;
    std::strncpy(m_name, name, sizeof(m_name) - 1);
    m_name[sizeof(m_name) - 1] = 0;
}

fuiRenderNodeCodeGenRect::~fuiRenderNodeCodeGenRect()
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
}

void fuiRenderNodeCodeGenRect::gather(bool recurse)
{
    (void)recurse;
}

void fuiRenderNodeCodeGenRect::preRender()
{
}

void fuiRenderNodeCodeGenRect::render(fuiRect* bounds, fuiMatrix* matrix)
{
    (void)bounds;
    (void)matrix;
    if (!isVisible())
        return;

    fuiRenderNode* root = getRoot();
    fuiFile* rootFile = root == 0 ? 0 : root->m_file;
    if (std::strncmp(m_name, "iggy_", 5) == 0 && rootFile != 0
        && rootFile->customDrawCallback != 0)
    {
        gRenderer->StateSetFaceCull(true);
        gRenderer->StateSetFaceCullCW(true);
        gRenderer->StateSetAlphaTestEnable(true);
        gRenderer->StateSetDepthTestEnable(true);
        gRenderer->StateSetDepthMask(true);
        gRenderer->StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
        pushMatrix();
        rootFile->customDrawCallback(rootFile->customDrawContext, m_name + 5, &m_bounds);
        popMatrix();
        gRenderer->StateSetFaceCull(false);
        gRenderer->StateSetBlendEnable(true);
        gRenderer->StateSetBlendFunc(1, 5);
        gRenderer->StateSetAlphaTestEnable(false);
        gRenderer->StateSetDepthTestEnable(false);
        gRenderer->StateSetDepthMask(false);
        return;
    }
    if (std::strncmp(m_name, "iggy_", 5) == 0)
        return;

    pushMatrix();
    pushColor();
    pushFilterFlags();
    const float width = m_bounds.xMax - m_bounds.xMin;
    const float height = m_bounds.yMax - m_bounds.yMin;
    const float right = m_bounds.xMin + width;
    const float bottom = m_bounds.yMin + height;
    float vertices[12] =
    {
        m_bounds.xMin, m_bounds.yMin,
        right, bottom,
        m_bounds.xMin, bottom,
        m_bounds.xMin, m_bounds.yMin,
        right, m_bounds.yMin,
        right, bottom
    };
    if (mTextureId == -1)
    {
        gRenderer->TextureBind(-1);
        gRenderer->StateSetColour(1.0f, 0.0f, 0.0f, 0.0f);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices,
                                C4JRender::eVertexType_UI, C4JRender::ePixelShaderType_UIColourAdd, NULL);
    }
    else
    {
        gRenderer->TextureBind(mTextureId);
        gRenderer->StateSetTextureScaleUI(1.0f / width, 1.0f / height,
                                          -m_bounds.xMin / width, -m_bounds.yMin / height);
        setRendererColour();
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices,
                                C4JRender::eVertexType_UI, C4JRender::ePixelShaderType_UIColourAdd, NULL);
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
