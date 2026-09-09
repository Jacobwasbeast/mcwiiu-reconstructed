#include "game/ui/FJ_SlotList_Slot.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

#include <cstdio>

#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_SlotList_Slot::FJ_SlotList_Slot(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_iSlotId(0), m_pRedBox(NULL),
      m_pSlotControl(NULL), m_pHighlight(NULL), m_bRedBoxActive(false),
      m_bHighlightActive(false)
{
    m_pRedBox = FJ_Factory::createObject("fourj.listClasses.FJ_ExlamationMark",
        pRenderNode, true, NULL);
    m_pSlotControl = getChildByName("IconHolder")->asMovieClip();
    m_pHighlight = getChildByName("Highlight")->asMovieClip();
    m_pHighlight->setVisible(false);
}

FJ_SlotList_Slot::~FJ_SlotList_Slot()
{
    if (m_pRedBox != NULL && m_pRedBox->getParent() != NULL)
        getParent()->removeChild(m_pRedBox, false);
}

void FJ_SlotList_Slot::SetParams(int iWidth, bool bVisible)
{
    m_pSlotControl->setVisible(!bVisible);
    setWidth((float)iWidth);
    m_pSlotControl->setWidth((float)iWidth);
}

void FJ_SlotList_Slot::SetSlotID(int iSlotId)
{
    m_iSlotId = iSlotId;
    int offset = Math::round(getScaleX());
    FJ_FuiNode* oldControl = m_pSlotControl->getChildByName("SlotControl")->asMovieClip();
    fuiRect rect;
    rect.xMin = (float)offset;
    rect.yMin = (float)offset;
    rect.xMax = getWidth() - (float)offset;
    rect.yMax = getHeight() - (float)offset;
    char name[64];
    std::sprintf(name, "iggy_slot_%d", iSlotId);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, name);
    renderNode->setAlpha(0.25f);
    removeChild(oldControl, true);
    FJ_FuiNode* replacement = new FJ_FuiNode(renderNode);
    FJ_FuiNode* parent = getParent();
    int index = parent->getNumChildren();
    parent->addChildAt(replacement, index);
}

void FJ_SlotList_Slot::SetHighlightActive(bool bActive)
{
    m_pHighlight->setVisible(bActive);
    m_bHighlightActive = bActive;
}

void FJ_SlotList_Slot::SetRedBoxActive(bool bActive)
{
    if (!bActive)
    {
        if (m_pRedBox->getParent() != NULL)
            getParent()->removeChild(m_pRedBox, false);
    }
    else if (m_pRedBox->getParent() == NULL)
    {
        FJ_FuiNode* parent = getParent();
        parent->addChild(m_pRedBox);
        m_pRedBox->setX(getX() + (float)Math::round(getScaleX()));
        m_pRedBox->setY(getY() + (float)Math::round(getScaleY()));
        m_pRedBox->setVisible(true);
    }
    m_pRedBox->setVisible(bActive);
    m_pSlotControl->gotoAndStop(bActive ? "SlotRed" : "SlotNormal");
    m_bRedBoxActive = bActive;
}

void FJ_SlotList_Slot::ShowSlot(bool bVisible)
{
    setVisible(bVisible);
}

FJ_SlotList_Slot* FJ_SlotList_Slot::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_SlotList_Slot(pRenderNode);
}

unsigned int FJ_SlotList_Slot::getType()
{
    return 0x5f;
}

int FJ_SlotList_Slot::getSlotId()
{
    return m_iSlotId;
}

bool FJ_SlotList_Slot::getHighLightActive()
{
    return m_bHighlightActive;
}

bool FJ_SlotList_Slot::getRedBoxActive()
{
    return m_bRedBoxActive;
}
