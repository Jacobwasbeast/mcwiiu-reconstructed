#include "game/ui/FJ_TexturePackList_Slot.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_TexturePackList_Slot::FJ_TexturePackList_Slot(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode), m_pSlotOutline(NULL), m_iSlotId(0)
{
    m_pSlotOutline = getChildByName("Slot_Outline")->asMovieClip();
    m_pSlotOutline->setVisible(false);
}

FJ_TexturePackList_Slot::~FJ_TexturePackList_Slot()
{
}

void FJ_TexturePackList_Slot::SetSelected(bool bSelected)
{
    m_pSlotOutline->setVisible(bSelected);
}

void FJ_TexturePackList_Slot::SetSlotID(int iSlotId)
{
    m_iSlotId = iSlotId;
}

void FJ_TexturePackList_Slot::SetTextureName(const std::wstring& textureName)
{
    FJ_FuiNode* slotPlaceHolder = getChildByName("SlotPlaceHolder")->asMovieClip();
    fuiRect rect;
    rect.xMin = slotPlaceHolder->getX();
    rect.yMin = slotPlaceHolder->getY();
    rect.xMax = rect.xMin + slotPlaceHolder->getWidth();
    rect.yMax = rect.yMin + slotPlaceHolder->getHeight();
    unsigned int width = 0;
    unsigned int height = 0;
    bool hasTexture = false;
    unsigned int texture = gFui->getCustomTexture(textureName, &width, &height, &hasTexture);
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(
        getStage()->m_pRenderNode, &rect, "CustomTexture", width, height,
        texture, true, hasTexture);
    FJ_FuiNode* customTexture = new FJ_FuiNode(renderNode);
    addChild(customTexture);
    removeChild(slotPlaceHolder, true);
}

FJ_TexturePackList_Slot* FJ_TexturePackList_Slot::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TexturePackList_Slot(pRenderNode);
}

unsigned int FJ_TexturePackList_Slot::getType()
{
    return 0x55;
}

int FJ_TexturePackList_Slot::getSlotId()
{
    return m_iSlotId;
}
