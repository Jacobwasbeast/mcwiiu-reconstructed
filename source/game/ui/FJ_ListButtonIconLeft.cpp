#include "game/ui/FJ_ListButtonIconLeft.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

class FJ_FuiNodeStage
{
public:
    fuiRenderNode* m_pRenderNode;
};

FJ_ListButtonIconLeft::FJ_ListButtonIconLeft(fuiRenderNode* renderNode)
    : FJ_Button(renderNode), m_pIconPlaceholder(NULL), m_pIconPlaceholder1(NULL)
{
}

FJ_ListButtonIconLeft::~FJ_ListButtonIconLeft()
{
}

void FJ_ListButtonIconLeft::Init(const std::wstring& name, int id)
{
    FJ_Base::SetLabelAlignment(4);
    m_isLabel = true;
    FJ_Button::Init(name, id);
}

void FJ_ListButtonIconLeft::SetSlotID(int slotId, bool hasSlot)
{
    FJ_FuiNode* slotControl = FJ_FuiNode::getChildByName("SlotControl")->asMovieClip();
    if (slotControl == NULL)
        return;

    if (!hasSlot)
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
    std::sprintf(name, "iggy_slot_%d_%d", slotId, hasSlot ? 1 : 0);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, name);
    renderNode->setAlpha(0.25f);
    removeChild(slotControl, true);
    addChild(new FJ_FuiNode(renderNode));
}

void FJ_ListButtonIconLeft::SetTextureName(const std::wstring& textureName,
                                           const std::wstring& textureName1)
{
    if (!textureName.empty())
    {
        if (m_pIconPlaceholder == NULL)
        {
            FJ_FuiNode* placeholder = getChildByName("IconPlaceholder")->asMovieClip();
            placeholder->setScaleX(placeholder->getScaleX() / getScaleX());
            placeholder->setScaleY(placeholder->getScaleY() / getScaleY());

            fuiRect rect;
            rect.xMin = 0.0f;
            rect.yMin = 0.0f;
            rect.xMax = placeholder->getWidth() / placeholder->getScaleX();
            rect.yMax = placeholder->getHeight() / placeholder->getScaleY();

            unsigned int width = 0;
            unsigned int height = 0;
            bool hasAlpha = false;
            unsigned int texture = gFui->getCustomTexture(
                textureName, &width, &height, &hasAlpha);
            fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
                getStage()->m_pRenderNode, &rect, "CustomTexture", width, height,
                texture, true, hasAlpha);
            m_pIconPlaceholder = new FJ_FuiNode(renderNode);
            placeholder->addChild(m_pIconPlaceholder);
        }
        else
        {
            unsigned int width = 0;
            unsigned int height = 0;
            bool hasAlpha = false;
            unsigned int texture = gFui->getCustomTexture(
                textureName, &width, &height, &hasAlpha);
            static_cast<fuiRenderNodeCodeGenRect*>(m_pIconPlaceholder->m_pRenderNode)->setTexture(
                width, height, texture, hasAlpha);
        }
    }

    if (!textureName1.empty())
    {
        if (m_pIconPlaceholder1 == NULL)
        {
            FJ_FuiNode* placeholder = getChildByName("IconPlaceholder1")->asMovieClip();
            placeholder->setScaleX(placeholder->getScaleX() / getScaleX());
            placeholder->setScaleY(placeholder->getScaleY() / getScaleY());

            fuiRect rect;
            rect.xMin = 0.0f;
            rect.yMin = 0.0f;
            rect.xMax = placeholder->getWidth() / placeholder->getScaleX();
            rect.yMax = placeholder->getHeight() / placeholder->getScaleY();

            unsigned int width = 0;
            unsigned int height = 0;
            bool hasAlpha = false;
            unsigned int texture = gFui->getCustomTexture(
                textureName1, &width, &height, &hasAlpha);
            fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
                getStage()->m_pRenderNode, &rect, "CustomTexture", width, height,
                texture, true, hasAlpha);
            m_pIconPlaceholder1 = new FJ_FuiNode(renderNode);
            placeholder->addChild(m_pIconPlaceholder1);
        }
        else
        {
            unsigned int width = 0;
            unsigned int height = 0;
            bool hasAlpha = false;
            unsigned int texture = gFui->getCustomTexture(
                textureName1, &width, &height, &hasAlpha);
            static_cast<fuiRenderNodeCodeGenRect*>(m_pIconPlaceholder1->m_pRenderNode)->setTexture(
                width, height, texture, hasAlpha);
        }
    }
}
