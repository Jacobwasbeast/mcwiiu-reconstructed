#include "game/ui/fuiRenderNodeBitmap.hpp"

#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"

fuiRenderNodeBitmap::fuiRenderNodeBitmap(fuiRenderNode* parent, fuiRenderNode* root, fuiObject* object,
                                         unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                         unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(parent, root, object, symbolIndex, eFuiObjectType_Bitmap, depth, color, clipDepth, visible, file)
{
    m_alpha = 1.0f;
    m_bounds.xMin = 0.0f;
    m_bounds.yMin = 0.0f;
    if (file != NULL && file->bitmaps != NULL && symbolIndex < (unsigned int)file->header.bitmapCount)
    {
        const fuiBitmap& bitmap = file->bitmaps[symbolIndex];
        m_bounds.xMax = (float)bitmap.width;
        m_bounds.yMax = (float)bitmap.height;
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

    if (m_file != NULL && m_file->bitmaps != NULL && m_symbolIndex < (unsigned int)m_file->header.bitmapCount)
    {
        fuiBitmap* bitmap = &m_file->bitmaps[m_symbolIndex];
        m_file->initialiseRenderTexture(bitmap, true);
        const float width = (float)bitmap->width;
        const float height = (float)bitmap->height;
        m_bounds.xMin = 0.0f;
        m_bounds.xMax = width;
        m_bounds.yMin = 0.0f;
        m_bounds.yMax = height;
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
