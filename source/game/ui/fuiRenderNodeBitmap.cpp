#include "game/ui/fuiRenderNodeBitmap.hpp"

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeBitmap::fuiRenderNodeBitmap(fuiRenderNode* parent, fuiRenderNode* root, fuiObject* object,
                                         unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                         unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(parent, root, object, symbolIndex, eFuiObjectType_Bitmap, depth, color, clipDepth, visible, file)
{
    m_alpha = 1.0f;
    fuiBitmap* bitmap = static_cast<fuiBitmap*>(m_object);
    m_bounds.xMin = 0.0f;
    m_bounds.xMax = (float)bitmap->width;
    m_bounds.yMin = 0.0f;
    m_bounds.yMax = (float)bitmap->height;
}

fuiRenderNodeBitmap::~fuiRenderNodeBitmap()
{
}

void fuiRenderNodeBitmap::gather(bool)
{
}

void fuiRenderNodeBitmap::preRender()
{
}

void fuiRenderNodeBitmap::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
    {
        return;
    }

    pushMatrix();
    pushColor();
    pushFilterFlags();

    {
        fuiBitmap* bitmap = static_cast<fuiBitmap*>(m_object);
        m_file->initialiseRenderTexture(bitmap, true);
        const float width = (float)bitmap->width;
        const float height = (float)bitmap->height;

        gRenderer->TextureBind(bitmap->textureId);
        gRenderer->StateSetTextureScaleUI(1.0f / width, 1.0f / height, 0.0f, 0.0f);

        float vertices[12] =
        {
            0.0f, 0.0f,
            width, height,
            0.0f, height,
            0.0f, 0.0f,
            width, 0.0f,
            width, height
        };

        if ((gFui->m_filterStack[gFui->m_filterDepth] & 1) == 0)
        {
            gRenderer->TextureSetParam(1, 0);
            gRenderer->TextureSetParam(2, 0);
        }
        else
        {
            gRenderer->TextureSetParam(1, 1);
            gRenderer->TextureSetParam(2, 1);
        }
        gRenderer->StateSetColour(gFui->m_colorStack[gFui->m_colorDepth].redMultiply, gFui->m_colorStack[gFui->m_colorDepth].greenMultiply, gFui->m_colorStack[gFui->m_colorDepth].blueMultiply, gFui->m_colorStack[gFui->m_colorDepth].alphaMultiply);
        gRenderer->StateSetColourAdd(gFui->m_colorStack[gFui->m_colorDepth].redAdd, gFui->m_colorStack[gFui->m_colorDepth].greenAdd, gFui->m_colorStack[gFui->m_colorDepth].blueAdd, gFui->m_colorStack[gFui->m_colorDepth].alphaAdd);
        gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, vertices,
                                C4JRender::eVertexType_UI, C4JRender::ePixelShaderType_UIColourAdd, NULL);
    }

    popFilterFlags();
    popColor();
    popMatrix();
}
