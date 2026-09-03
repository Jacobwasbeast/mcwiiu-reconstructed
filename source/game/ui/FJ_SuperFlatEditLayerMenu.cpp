#include "game/ui/FJ_SuperFlatEditLayerMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_SuperFlatEditLayerMenu::FJ_SuperFlatEditLayerMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode), mSlotID(0), mScrollUp(NULL), mScrollDown(NULL),
      mScrollTop(0), mScrollBottom(0)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    if (scrollBar != NULL)
    {
        mScrollUp = new FJ_ScrollArrow(renderNode);
        mScrollDown = new FJ_ScrollArrow(renderNode);
        scrollBar->addChild(mScrollUp);
        scrollBar->addChild(mScrollDown);
        mScrollUp->ChangeState(0);
        mScrollDown->ChangeState(3);
        mScrollUp->setVisible(false);
        mScrollDown->setVisible(false);
        FJ_FuiNode* track = scrollBar->getChildByName(std::string("ScrollBarTrack"));
        FJ_FuiNode* button = scrollBar->getChildByName(std::string("ScrollBarButton"));
        if (track != NULL && button != NULL)
        {
            mScrollTop = (int)button->getY();
            mScrollBottom = (int)(track->getY() + track->getHeight() - button->getHeight());
        }
    }
    SetSlotID();
}

FJ_SuperFlatEditLayerMenu::~FJ_SuperFlatEditLayerMenu()
{
}

void FJ_SuperFlatEditLayerMenu::SetSlotID()
{
    FJ_FuiNode* panel = getChildByName(std::string("MainPanel"));
    if (panel != NULL)
    {
        FJ_FuiNode* slotControl = panel->getChildByName(std::string("SlotControl"));
        if (slotControl != NULL)
        {
            slotControl->setVisible(true);
        }
    }
}

FJ_AbstractContainerDocument* FJ_SuperFlatEditLayerMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SuperFlatEditLayerMenu(renderNode);
}

unsigned int FJ_SuperFlatEditLayerMenu::getType()
{
    return 41;
}

void FJ_SuperFlatEditLayerMenu::SetScrollBar(int count, int position)
{
    if (mScrollUp == NULL || mScrollDown == NULL)
    {
        return;
    }
    if (count <= 1)
    {
        mScrollUp->setVisible(false);
        mScrollDown->setVisible(false);
        mSlotID = 0;
        return;
    }
    if (position < 0)
    {
        position = 0;
    }
    if (position >= count)
    {
        position = count - 1;
    }
    mScrollUp->setVisible(position > 0);
    mScrollDown->setVisible(position + 1 < count);
    if (position > 0)
    {
        mScrollUp->ChangeState(position + 1 < count ? 1 : 0);
    }
    if (position + 1 < count)
    {
        mScrollDown->ChangeState(position > 0 ? 4 : 3);
    }
    mSlotID = position;
}
