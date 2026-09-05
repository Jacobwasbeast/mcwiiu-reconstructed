#include "game/ui/fuiRenderNodeShape.hpp"

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiShape.hpp"
#include "game/ui/fuiShapeComponent.hpp"
#include "game/ui/fuiVert.hpp"

extern Renderer* gRenderer;

fuiRenderNodeShape::fuiRenderNodeShape(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object,
                                       unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                       unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(stage, parent, object, symbolIndex, eFuiObjectType_Shape, depth, color, clipDepth, visible, file)
{
    fuiShape* shape = static_cast<fuiShape*>(m_object);
    m_bounds = shape->bounds;
}

fuiRenderNodeShape::~fuiRenderNodeShape()
{
}

void fuiRenderNodeShape::gather(bool recurse)
{
}

void fuiRenderNodeShape::preRender()
{
}

void fuiRenderNodeShape::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
    {
        return;
    }

    pushMatrix();
    pushColor();
    pushFilterFlags();

    fuiShape* shape = static_cast<fuiShape*>(m_object);
    fuiShapeComponent* component = m_file->shapeComponentData + shape->shapeComponentIndex;
    unsigned int i;
    for (i = 0; i < shape->shapeComponentCount; ++i, ++component)
    {
        unsigned int vertexOffset = component->vertexCount;
        unsigned int vertexCount = component->vertexOffset;
        if (vertexCount != 0)
        {
            C4JRender::eVertexType vertexType = C4JRender::eVertexType_UI;
            C4JRender::ePixelShaderType pixelShaderType = C4JRender::ePixelShaderType_UIColourAdd;
            if (component->fillType == (eFuiFillType)1)
            {
                unsigned int rgba = component->color.rgba;
                gRenderer->TextureBind(-1);
                gRenderer->StateSetColour(((rgba >> 24) & 0xff) / 255.0f * m_redMultiply,
                                          ((rgba >> 16) & 0xff) / 255.0f * m_greenMultiply,
                                          ((rgba >> 8) & 0xff) / 255.0f * m_blueMultiply,
                                          (rgba & 0xff) / 255.0f * m_alpha);
                gRenderer->StateSetColourAdd(m_redAdd, m_greenAdd, m_blueAdd, m_alphaAdd);
            }
            else
            {
                fuiBitmap* bitmap = m_file->bitmaps + component->bitmapId;
                m_file->initialiseRenderTexture(bitmap, true);
                gRenderer->TextureBind(bitmap->textureId);
                if (component->fillType == (eFuiFillType)3)
                {
                    fuiMatrix scaleToPixels = {32768.0f, 0.0f, 0.0f, 32768.0f, -16384.0f, -16384.0f};
                    fuiMatrix scaleToTwips = {0.05f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f};
                    fuiMatrix bitmapScale;
                    fuiMatrix textureMatrix;
                    fuiMatrix::mul(scaleToPixels, scaleToTwips, bitmapScale);
                    fuiMatrix::mul(bitmapScale, component->bitmapMatrix, textureMatrix);
                    gRenderer->StateSetTextureScaleUI(1.0f / textureMatrix.scaleX,
                                                      1.0f / textureMatrix.scaleY,
                                                      -textureMatrix.translateX / textureMatrix.scaleX,
                                                      -textureMatrix.translateY / textureMatrix.scaleY);
                    vertexType = C4JRender::eVertexType_TiledUI;
                    pixelShaderType = C4JRender::ePixelShaderType_UIAlphaTest;
                }
                else
                {
                    float width = (float)bitmap->width;
                    float height = (float)bitmap->height;
                    gRenderer->StateSetTextureScaleUI(1.0f / width, 1.0f / height,
                                                      -component->bitmapMatrix.translateX / width,
                                                      -component->bitmapMatrix.translateY / height);
                }
                gRenderer->TextureSetParam(2, m_renderFlags & 1);
                gRenderer->StateSetColour(m_redMultiply, m_greenMultiply, m_blueMultiply, m_alpha);
                gRenderer->StateSetColourAdd(m_redAdd, m_greenAdd, m_blueAdd, m_alphaAdd);
            }
            if (bounds != NULL)
            {
                gRenderer->StateSetScalingGridUI(shape->bounds.xMin, bounds->xMin, bounds->xMax,
                                                 shape->bounds.xMax, shape->bounds.yMin, bounds->yMin,
                                                 bounds->yMax, shape->bounds.yMax,
                                                 matrix->scaleX, matrix->scaleY);
                vertexType = C4JRender::eVertexType_ScalingGridUI;
            }
            gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, vertexCount,
                                    m_file->vertexData + vertexOffset, vertexType,
                                    pixelShaderType, NULL);
        }
    }

    popFilterFlags();
    popColor();
    popMatrix();
}
