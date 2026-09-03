#include "game/ui/fuiRenderNodeBitmap.hpp"

#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeBitmap::fuiRenderNodeBitmap(fuiRenderNode* parent, fuiRenderNode* root, fuiObject* object,
                                         unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                         unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(parent, root, object, symbolIndex, eFuiObjectType_Bitmap, depth, color, clipDepth, visible, file)
{
    m_colorTransform = 0;
    m_localScaleX = 1.0f;
    m_localScaleY = 1.0f;
    m_boundsXMin = 0.0f;
    m_boundsYMin = 0.0f;
    if (file != NULL && file->bitmaps != NULL && symbolIndex < (unsigned int)file->bitmapCount)
    {
        const fuiBitmap& bitmap = file->bitmaps[symbolIndex];
        m_boundsXMax = (float)bitmap.width;
        m_boundsYMax = (float)bitmap.height;
        m_width = bitmap.width;
    }
}

fuiRenderNodeBitmap::~fuiRenderNodeBitmap()
{
}

void fuiRenderNodeBitmap::gather(bool recurse)
{
    if (recurse)
    {
        progogateBounds();
    }
}

void fuiRenderNodeBitmap::preRender()
{
    generateGlobalMatrix();
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

    if (m_file != NULL && m_file->bitmaps != NULL && m_symbolIndex < (unsigned int)m_file->bitmapCount)
    {
        fuiBitmap* bitmap = &m_file->bitmaps[m_symbolIndex];
        m_file->initialiseRenderTexture(bitmap, true);
        const float width = (float)bitmap->width;
        const float height = (float)bitmap->height;
        m_boundsXMin = 0.0f;
        m_boundsXMax = width;
        m_boundsYMin = 0.0f;
        m_boundsYMax = height;
        m_width = bitmap->width;
        if (bounds != NULL)
        {
            bounds->xMin = 0.0f;
            bounds->xMax = width;
            bounds->yMin = 0.0f;
            bounds->yMax = height;
        }
    }

    popFilterFlags();
    popColor();
    popMatrix();
}
