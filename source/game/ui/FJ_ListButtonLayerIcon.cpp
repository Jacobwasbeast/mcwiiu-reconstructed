#include "game/ui/FJ_ListButtonLayerIcon.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

extern fui* gFui;

FJ_ListButtonLayerIcon::FJ_ListButtonLayerIcon(fuiRenderNode* pRenderNode)
    : FJ_Button(pRenderNode), m_textureNode(NULL)
{
}

FJ_ListButtonLayerIcon::~FJ_ListButtonLayerIcon()
{
}

void FJ_ListButtonLayerIcon::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_isLabel = true;
    FJ_Button::Init(name, id);
}

void FJ_ListButtonLayerIcon::SetSlotID(int slotId, bool visible)
{
    FJ_FuiNode* slotControl = getChildByName("SlotControl")->asMovieClip();
    if (slotControl != NULL)
    {
        if (!visible)
        {
            removeChild(slotControl, true);
            return;
        }

        fuiRect rect;
        rect.xMin = slotControl->getX();
        rect.yMin = slotControl->getY();
        rect.xMax = rect.xMin + slotControl->getWidth() / getScaleX();
        rect.yMax = rect.yMin + slotControl->getHeight();

        char name[64];
        std::sprintf(name, "iggy_slot_%d_%d", slotId, visible ? 1 : 0);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
            getStage()->m_pRenderNode, &rect, name);
        renderNode->setAlpha(0.25f);
        removeChild(slotControl, true);
        slotControl = new FJ_FuiNode(renderNode);
        addChild(slotControl);
    }
}

void FJ_ListButtonLayerIcon::SetTextureName(const std::wstring& textureName,
                                            const std::wstring& layerName)
{
    if (!textureName.empty() && m_textureNode == NULL)
    {
        FJ_FuiNode* iconPlaceholder = getChildByName("IconPlaceholder")->asMovieClip();
        iconPlaceholder->setScaleX(iconPlaceholder->getScaleX() / getScaleX());
        iconPlaceholder->setScaleY(iconPlaceholder->getScaleY() / getScaleY());

        fuiRect rect;
        rect.xMin = 0.0f;
        rect.yMin = 0.0f;
        rect.xMax = iconPlaceholder->getWidth() / iconPlaceholder->getScaleX();
        rect.yMax = iconPlaceholder->getHeight() / iconPlaceholder->getScaleY();

        unsigned int width = 0;
        unsigned int height = 0;
        bool hasTexture = false;
        unsigned int texture = gFui->getCustomTexture(textureName, &width, &height, &hasTexture);
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
            getStage()->m_pRenderNode, &rect, "CustomTexture", width, height,
            texture, true, hasTexture);
        m_textureNode = new FJ_FuiNode(renderNode);
        iconPlaceholder->addChild(m_textureNode);
    }
}
