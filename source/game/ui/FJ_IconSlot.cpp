#include "game/ui/FJ_IconSlot.hpp"

#include <cstdio>

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/util/Math.hpp"

FJ_IconSlot::FJ_IconSlot(fuiRenderNode* pRenderNode)
    : FJ_Base(pRenderNode)
{
}

FJ_IconSlot::~FJ_IconSlot()
{
}

FJ_Base* FJ_IconSlot::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_IconSlot(pRenderNode);
}

unsigned int FJ_IconSlot::getType()
{
    return 0x800b;
}

void FJ_IconSlot::ShowSlotHolder(bool bVisible)
{
    m_pIconHolder->setVisible(bVisible);
}

void FJ_IconSlot::ShowRedSlot(bool bRed)
{
    m_pIconHolder->gotoAndStop(bRed ? "SlotRed" : "SlotNormal");
}

void FJ_IconSlot::SetParams(int iWidth, bool bUseSmallText)
{
    m_pIconHolder = getChildByName("IconHolder")->asMovieClip();
    m_pBitmapIcon = getChildByName("BitmapIcon")->asFJ_BitmapIcon();

    if (!bUseSmallText)
    {
        removeChild(getChildByName("SmallTextContainer"), true);
    }
    else
    {
        removeChild(getChildByName("FJ_TextContainer"), true);
        FJ_FuiNode* smallText = getChildByName("SmallTextContainer")->asMovieClip();
        addChild(smallText->getChildAt(0)->asMovieClip());
        removeChild(getChildByName("SmallTextContainer"), true);
    }

    if (iWidth > 0)
    {
        setWidth((float)iWidth);
        setWidth((float)iWidth);
    }
    ShowRedSlot(false);
}

void FJ_IconSlot::Init(const std::wstring& label)
{
    SetLabelAlignment(2);
    InitBase(label, -1);
}

void FJ_IconSlot::SetLabel(const std::wstring& label)
{
    SetLabelAlignment(2);
    m_isLabel = true;
    FJ_Base::SetLabel(label);
}

void FJ_IconSlot::SetSlotID(const std::string& slotName, int iSlotId)
{
    m_iSlotId = iSlotId;
    FJ_FuiNode* oldControl = m_pIconHolder->getChildByName("SlotControl");
    if (oldControl == NULL)
        return;

    float scale = getScaleX();
    int roundedScale = Math::round(scale);
    float offset = (float)roundedScale / scale;
    float extent = offset + (float)Math::round(getWidth() - 2.0f * roundedScale) / scale;
    fuiRect rect;
    rect.xMin = offset;
    rect.yMin = offset;
    rect.xMax = extent;
    rect.yMax = extent;

    char name[64];
    std::sprintf(name, "iggy_slot_%s_%d", slotName.c_str(), iSlotId);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, name);
    renderNode->setAlpha(0.25f);
    m_pIconHolder->removeChild(oldControl, true);
    addChild(new FJ_FuiNode(renderNode));
}

void FJ_IconSlot::SetTextureName(const std::wstring& textureName)
{
    m_pBitmapIcon->SetTextureName(textureName);
}

int FJ_IconSlot::getSlotId()
{
    return m_iSlotId;
}
