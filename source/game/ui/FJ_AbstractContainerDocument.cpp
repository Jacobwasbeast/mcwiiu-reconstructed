#include "game/ui/FJ_AbstractContainerDocument.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_AbstractContainerDocument::FJ_AbstractContainerDocument(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_AbstractContainerDocument::~FJ_AbstractContainerDocument()
{
}

void FJ_AbstractContainerDocument::handleMouseMove(FJ_Event* event)
{
    if (event == 0)
    {
        return;
    }
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"));
    if (mainPanel == 0)
    {
        return;
    }
    FJ_FuiNode* cursor = mainPanel->getChildByName(std::string("fcursor"));
    if (cursor != 0)
    {
        cursor->setVisible(true);
    }
}

void FJ_AbstractContainerDocument::_handleMouseMove(FJ_FuiNode* node, FJ_Event* event)
{
    if (node != 0)
    {
        static_cast<FJ_AbstractContainerDocument*>(node)->handleMouseMove(event);
    }
}

void FJ_AbstractContainerDocument::init(FJ_Event* event)
{
    FJ_Document::init(event);
    if (event != 0)
    {
        eventListen(event, false);
    }
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"));
    if (mainPanel != 0)
    {
        FJ_FuiNode* cursor = mainPanel->getChildByName(std::string("cursor"));
        if (cursor != 0)
        {
            cursor->setVisible(true);
        }
    }
}

void FJ_AbstractContainerDocument::HideInventory()
{
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_SlotList* inventory = mainPanel->getChildByName(std::string("inventoryList"))->asFJ_SlotList();
    FJ_SlotList* hotbar = mainPanel->getChildByName(std::string("hotbarList"))->asFJ_SlotList();

    if (mainPanel != 0 && inventory != 0 && hotbar != 0)
    {
        fuiRect bounds;
        mainPanel->getRenderNodeChildAt(0)->getParentSpaceBounds(&bounds);
        float oldY = bounds.yMin;

        FJ_FuiNode* backgroundPanel = mainPanel->getChildByName(std::string("BackgroundPanel"))->asMovieClip();
        FJ_FuiNode* backgroundChild = backgroundPanel->getChildAt(0);
        if (backgroundChild == 0)
        {
            mainPanel->getChildAt(0)->decHeight(hotbar->getY() - inventory->getY());
        }
        else
        {
            backgroundPanel->getChildAt(0)->decHeight(hotbar->getY() - inventory->getY());
        }

        hotbar->setY(inventory->getY());
        mainPanel->removeChild(inventory, true);

        mainPanel->getRenderNodeChildAt(0)->getParentSpaceBounds(&bounds);
        float newY = bounds.yMin;
        float delta = (float)Math::round(((176.0f - oldY) - newY) * 0.5f);
        incY(176.0f - delta);
    }
}
