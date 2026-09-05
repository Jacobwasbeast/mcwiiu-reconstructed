#include "game/ui/fuiRenderNodeShape.hpp"

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"

extern Renderer* gRenderer;

struct fuiShapeObject : fuiObject
{
    unsigned int componentIndex;
    unsigned int componentCount;
    float xMin;
    float xMax;
    float yMin;
    float yMax;
};

struct fuiShapeComponent
{
    unsigned int type;
    unsigned int bitmapIndex;
    unsigned int matrixIndex;
    fuiMatrix matrix;
    unsigned int vertexIndex;
    unsigned int vertexCount;
};

fuiRenderNodeShape::fuiRenderNodeShape(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object,
                                       unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                       unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(stage, parent, object, symbolIndex, eFuiObjectType_Shape, depth, color, clipDepth, visible, file)
{
    fuiShapeObject* shape = (fuiShapeObject*)object;
    if (shape != NULL)
    {
        m_boundsXMin = shape->xMin;
        m_boundsXMax = shape->xMax;
        m_boundsYMin = shape->yMin;
        m_boundsYMax = shape->yMax;
    }
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

    fuiShapeObject* shape = (fuiShapeObject*)m_node;
    if (shape != NULL && m_file != NULL && m_file->shapeComponentData != 0 && gRenderer != NULL)
    {
        unsigned int i;
        fuiShapeComponent* component = (fuiShapeComponent*)m_file->shapeComponentData;
        component += shape->componentIndex;
        for (i = 0; i < shape->componentCount; ++i, ++component)
        {
            if (component->vertexCount != 0)
            {
                unsigned int vertexType = C4JRender::eVertexType_UI;
                unsigned int pixelShader = C4JRender::ePixelShaderType_UI;
                if (component->bitmapIndex < (unsigned int)m_file->bitmapCount && m_file->bitmaps != NULL)
                {
                    fuiBitmap* bitmap = &m_file->bitmaps[component->bitmapIndex];
                    m_file->initialiseRenderTexture(bitmap, true);
                    gRenderer->TextureBind(bitmap->textureId);
                }
                if (component->type == 3)
                {
                    fuiMatrix identity;
                    fuiMatrix scaleGrid;
                    fuiMatrix transform;
                    identity.scaleX = 0.05f;
                    identity.rotateSkew0 = 0.0f;
                    identity.rotateSkew1 = 0.0f;
                    identity.scaleY = 0.05f;
                    identity.translateX = 0.0f;
                    identity.translateY = 0.0f;
                    scaleGrid.scaleX = 32768.0f;
                    scaleGrid.rotateSkew0 = 0.0f;
                    scaleGrid.rotateSkew1 = 0.0f;
                    scaleGrid.scaleY = 32768.0f;
                    scaleGrid.translateX = -16384.0f;
                    scaleGrid.translateY = -16384.0f;
                    fuiMatrix::mul(scaleGrid, identity, transform);
                    fuiMatrix::mul(transform, component->matrix, scaleGrid);
                    gRenderer->StateSetTextureScaleUI(1.0f / scaleGrid.scaleX, 1.0f / scaleGrid.scaleY,
                                                      -scaleGrid.translateX / scaleGrid.scaleX,
                                                      -scaleGrid.translateY / scaleGrid.scaleY);
                    vertexType = C4JRender::eVertexType_ScalingGridUI;
                    pixelShader = C4JRender::ePixelShaderType_UIAlphaTest;
                }
                else
                {
                    gRenderer->StateSetTextureScaleUI(1.0f, 1.0f, -component->matrix.translateX,
                                                      -component->matrix.translateY);
                }
                gRenderer->StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
                gRenderer->StateSetColourAdd(0.0f, 0.0f, 0.0f, 0.0f);
                gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles,
                                        component->vertexCount,
                                        (void*)((unsigned char*)m_file->vertexData + component->vertexIndex * 8),
                                        (C4JRender::eVertexType)vertexType,
                                        (C4JRender::ePixelShaderType)pixelShader, NULL);
            }
        }
    }

    popFilterFlags();
    popColor();
    popMatrix();
}
