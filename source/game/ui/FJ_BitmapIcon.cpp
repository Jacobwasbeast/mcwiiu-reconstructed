#include "game/ui/FJ_BitmapIcon.hpp"

#include "game/ui/fuiRenderNode.hpp"

class fuiRenderNodeCodeGenRect : public fuiRenderNode
{
public:
    fuiRenderNodeCodeGenRect(fuiRenderNode* pRenderNode, fuiRect* rect,
        const char* textureName, unsigned int width, unsigned int height,
        int texture, bool tiled, bool filtered);
    void setTexture(unsigned int width, unsigned int height, int texture, bool filtered);
};

class fui
{
public:
    unsigned int getCustomTexture(const std::wstring& textureName,
        unsigned int* width, unsigned int* height, bool* hasTexture);
};

extern fui* g_fui;

FJ_BitmapIcon::FJ_BitmapIcon(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_pBitmapNode(NULL)
{
}

FJ_BitmapIcon::~FJ_BitmapIcon()
{
}

FJ_FuiNode* FJ_BitmapIcon::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_BitmapIcon(pRenderNode);
}

unsigned int FJ_BitmapIcon::getType()
{
    return 0x67;
}

void FJ_BitmapIcon::SetTextureName(const std::wstring& textureName)
{
    if (textureName.empty())
    {
        if (m_pBitmapNode != NULL)
            m_pBitmapNode->setVisible(false);
        return;
    }

    if (m_pBitmapNode != NULL)
    {
        if (!isVisible())
            m_pBitmapNode->setVisible(true);

        unsigned int width = 0;
        unsigned int height = 0;
        bool hasTexture = false;
        unsigned int texture = g_fui->getCustomTexture(textureName, &width, &height, &hasTexture);
        static_cast<fuiRenderNodeCodeGenRect*>(m_pBitmapNode->m_pRenderNode)->setTexture(
            width, height, texture, hasTexture);
        return;
    }

    fuiRect rect;
    rect.xMin = 0.0f;
    rect.xMax = getWidth() / getScaleX();
    rect.yMin = 0.0f;
    rect.yMax = getHeight() / getScaleY();
    unsigned int width = 0;
    unsigned int height = 0;
    bool hasTexture = false;
    unsigned int texture = g_fui->getCustomTexture(textureName, &width, &height, &hasTexture);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
        m_pRenderNode, &rect, "CustomTexture", width, height, texture, true, hasTexture);
    m_pBitmapNode = new FJ_FuiNode(renderNode);
    addChild(m_pBitmapNode);
}
