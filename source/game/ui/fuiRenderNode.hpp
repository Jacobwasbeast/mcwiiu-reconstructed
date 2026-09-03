#ifndef GAME_UI_FUIRENDERNODE_HPP
#define GAME_UI_FUIRENDERNODE_HPP

#include "game/ui/fuiMatrix.hpp"
#include "game/ui/fuiObject.hpp"
#include "game/ui/fuiRGBA.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiSymbol.hpp"

class FJ_FuiNode;
class fuiFile;
class fuiRenderNodeStage;

class fuiRenderNode
{
protected:
    unsigned int m_flags;
    unsigned int m_symbolIndex;
    eFuiObjectType m_type;
    unsigned int m_colorTransform;
    unsigned char m_visible;
    float m_localScaleX;
    float m_localSkewX;
    float m_localSkewY;
    float m_localScaleY;
    float m_localX;
    float m_localY;
    float m_globalScaleX;
    float m_globalSkewX;
    float m_globalSkewY;
    float m_globalScaleY;
    float m_globalX;
    float m_globalY;
    float m_alpha;
    unsigned int m_frame;
    unsigned int m_depth;
    unsigned int m_clipDepth;
    unsigned int m_filterFlags;
    float m_boundsXMin;
    float m_boundsXMax;
    float m_boundsYMin;
    float m_boundsYMax;
    unsigned int m_width;
    unsigned char m_hasScrollRect;
    fuiRenderNodeStage* m_stage;
    fuiRenderNode* m_parent;
    FJ_FuiNode* m_node;
    fuiFile* m_file;

public:
    fuiRenderNode(fuiRenderNode*, fuiRenderNode*, fuiObject*, unsigned int, eFuiObjectType, unsigned char, fuiRGBA*, unsigned char, bool, fuiFile*);
    virtual ~fuiRenderNode();
    virtual void shutdown();
    virtual void gather(bool) = 0;
    virtual void preRender() = 0;
    virtual void render(fuiRect*, fuiMatrix*) = 0;
    virtual void handleAddedToStage();
    virtual fuiRenderNode* findNode(const char*);
    virtual fuiRenderNode* getNodeFromPath(const char*);
    virtual void setVisibility(bool);
    virtual void setX(float);
    virtual void setY(float);
    virtual void setWidth(float);
    virtual void setHeight(float);
    virtual float getWidth();
    virtual float getHeight();
    virtual void getParentSpaceBounds(fuiRect*);
    virtual void generateGlobalMatrix();
    void disableCtor();
    void pushMatrix(fuiMatrix&);
    void setScrollRectViewport();
    void popMatrix();
    void clearScrollRectViewport();
    bool isVisible();
    void setAlpha(float);
    float getAlpha();
    fuiRenderNode* asTimeline();
    void progogateBounds();
    float getX();
    float getY();
    float getScaleX();
    void setScaleX(float);
    float getScaleY();
    void setScaleY(float);
    fuiRenderNodeStage* getStage();
    FJ_FuiNode* getRoot();
    void pushMatrix();
    void pushColor();
    void popColor();
    void pushFilterFlags();
    void popFilterFlags();
    fuiRenderNode* asEditText();
    fuiRect getGlobalBounds() const;
    void setGlobalBounds(const fuiRect&);
    fuiRenderNode* asShape();
    fuiRenderNode* asBitmap();
};

#endif
