#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"

#include <cmath>
#include <cstring>

fuiRenderNode::fuiRenderNode(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object, unsigned int symbolIndex, eFuiObjectType type, unsigned char colorTransform, fuiRGBA* color, unsigned char visible, bool useDefault, fuiFile* file)
    : m_flags(0), m_symbolIndex(symbolIndex), m_type(type), m_colorTransform(colorTransform), m_visible(visible ? 1 : 0), m_localScaleX(1.0f), m_localSkewX(0.0f), m_localSkewY(0.0f), m_localScaleY(1.0f), m_localX(0.0f), m_localY(0.0f), m_globalScaleX(1.0f), m_globalSkewX(0.0f), m_globalSkewY(0.0f), m_globalScaleY(1.0f), m_globalX(0.0f), m_globalY(0.0f), m_alpha(1.0f), m_frame(0), m_depth(0), m_clipDepth(0), m_filterFlags(0), m_boundsXMin(0.0f), m_boundsXMax(0.0f), m_boundsYMin(0.0f), m_boundsYMax(0.0f), m_width(0), m_hasScrollRect(0), m_stage(0), m_parent(parent), m_node(0), m_file(0)
{
    m_stage = (fuiRenderNodeStage*)stage;
    m_file = file;
    m_node = (FJ_FuiNode*)object;
    if (color != 0) m_colorTransform = color->rgba;
    if (useDefault) m_flags |= 0x100;
    generateGlobalMatrix();
}

fuiRenderNode::~fuiRenderNode()
{
    m_node = 0;
}

void fuiRenderNode::shutdown() {}

void fuiRenderNode::handleAddedToStage()
{
    if (m_node != 0)
    {
        FJ_Event event(eFJEventType_Init, false, false);
        m_node->dispatchEvent(&event);
    }
}

fuiRenderNode* fuiRenderNode::findNode(const char* name)
{
    if (name == 0 || name[0] == 0) return this;
    return 0;
}

fuiRenderNode* fuiRenderNode::getNodeFromPath(const char* path)
{
    if (m_node != 0 && path != 0)
    {
        if (m_node->getName() == path || std::strcmp(path, "?") == 0)
        {
            return this;
        }
    }
    return 0;
}

void fuiRenderNode::generateGlobalMatrix()
{
    if (m_parent != 0) {
        fuiMatrix local = { m_localScaleX, m_localSkewX, m_localSkewY, m_localScaleY, m_localX, m_localY };
        fuiMatrix parent = { m_parent->m_globalScaleX, m_parent->m_globalSkewX, m_parent->m_globalSkewY, m_parent->m_globalScaleY, m_parent->m_globalX, m_parent->m_globalY };
        fuiMatrix result;
        fuiMatrix::mul(local, parent, result);
        m_globalScaleX = result.scaleX; m_globalSkewX = result.rotateSkew0; m_globalSkewY = result.rotateSkew1; m_globalScaleY = result.scaleY; m_globalX = result.translateX; m_globalY = result.translateY;
    } else {
        m_globalScaleX = m_localScaleX; m_globalSkewX = m_localSkewX; m_globalSkewY = m_localSkewY; m_globalScaleY = m_localScaleY; m_globalX = m_localX; m_globalY = m_localY;
    }
}

void fuiRenderNode::disableCtor() { m_flags &= ~1u; }
void fuiRenderNode::pushMatrix(fuiMatrix& matrix) { fuiMatrix::mul(matrix, matrix, matrix); }
void fuiRenderNode::setScrollRectViewport() { if (m_boundsXMax != m_boundsXMin || m_boundsYMax != m_boundsYMin) pushMatrix(); }
void fuiRenderNode::popMatrix() {}
void fuiRenderNode::clearScrollRectViewport() { if (m_boundsXMax != m_boundsXMin || m_boundsYMax != m_boundsYMin) popMatrix(); }
void fuiRenderNode::setVisibility(bool visible) { if (visible) m_flags |= 8u; else m_flags &= ~8u; }
bool fuiRenderNode::isVisible() { return (m_flags & 8u) != 0; }
void fuiRenderNode::setAlpha(float alpha) { m_alpha = alpha; }
float fuiRenderNode::getAlpha() { return m_alpha; }
fuiRenderNode* fuiRenderNode::asTimeline() { return m_type == eFuiObjectType_Timeline ? this : 0; }
void fuiRenderNode::progogateBounds() { generateGlobalMatrix(); }
void fuiRenderNode::setX(float x) { m_localX = x; progogateBounds(); }
void fuiRenderNode::setY(float y) { m_localY = y; progogateBounds(); }
float fuiRenderNode::getX() { return m_localX; }
float fuiRenderNode::getY() { return m_localY; }
float fuiRenderNode::getScaleX() { return std::sqrt(m_localScaleX * m_localScaleX + m_localSkewX * m_localSkewX); }
void fuiRenderNode::setScaleX(float scale) { float old = getScaleX(); if (old != 0.0f) { m_localScaleX *= scale / old; m_localSkewX *= scale / old; } progogateBounds(); }
float fuiRenderNode::getScaleY() { return std::sqrt(m_localScaleY * m_localScaleY + m_localSkewY * m_localSkewY); }
void fuiRenderNode::setScaleY(float scale) { float old = getScaleY(); if (old != 0.0f) { m_localScaleY *= scale / old; m_localSkewY *= scale / old; } progogateBounds(); }
void fuiRenderNode::setWidth(float width) { if (width > 0.0f) setScaleX(getScaleX() * width / (m_boundsXMax - m_boundsXMin)); }
void fuiRenderNode::setHeight(float height) { if (height > 0.0f) setScaleY(getScaleY() * height / (m_boundsYMax - m_boundsYMin)); }
float fuiRenderNode::getWidth() { return m_boundsXMax - m_boundsXMin; }
float fuiRenderNode::getHeight() { return m_boundsYMax - m_boundsYMin; }
void fuiRenderNode::getParentSpaceBounds(fuiRect* rect)
{
    if (rect != 0)
    {
        rect->xMin = m_globalX + m_boundsXMin * m_globalScaleX;
        rect->xMax = m_globalX + m_boundsXMax * m_globalScaleX;
        rect->yMin = m_globalY + m_boundsYMin * m_globalScaleY;
        rect->yMax = m_globalY + m_boundsYMax * m_globalScaleY;
    }
}
fuiRenderNodeStage* fuiRenderNode::getStage() { return m_stage; }
FJ_FuiNode* fuiRenderNode::getRoot() { return m_node; }
void fuiRenderNode::pushMatrix() {}
void fuiRenderNode::pushColor() {}
void fuiRenderNode::popColor() {}
void fuiRenderNode::pushFilterFlags() { ++m_filterFlags; }
void fuiRenderNode::popFilterFlags() { if (m_filterFlags != 0) --m_filterFlags; }
fuiRenderNode* fuiRenderNode::asEditText() { return m_type == eFuiObjectType_EditText ? this : 0; }
fuiRect fuiRenderNode::getGlobalBounds() const { fuiRect rect = { m_globalX + m_boundsXMin, m_globalX + m_boundsXMax, m_globalY + m_boundsYMin, m_globalY + m_boundsYMax }; return rect; }
void fuiRenderNode::setGlobalBounds(const fuiRect& rect) { m_boundsXMin = rect.xMin; m_boundsXMax = rect.xMax; m_boundsYMin = rect.yMin; m_boundsYMax = rect.yMax; }
fuiRenderNode* fuiRenderNode::asShape() { return m_type == eFuiObjectType_Shape ? this : 0; }
fuiRenderNode* fuiRenderNode::asBitmap() { return m_type == eFuiObjectType_Bitmap ? this : 0; }
