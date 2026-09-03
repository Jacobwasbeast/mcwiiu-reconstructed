#include "game/ui/FJ_AbstractContainerDocument.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"

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
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"));
    if (mainPanel == 0)
    {
        return;
    }
    FJ_FuiNode* inventory = mainPanel->getChildByName(std::string("inventoryList"));
    FJ_FuiNode* hotbar = mainPanel->getChildByName(std::string("hotbarList"));
    if (inventory != 0)
    {
        mainPanel->removeChild(inventory, true);
    }
    if (hotbar != 0)
    {
        mainPanel->removeChild(hotbar, true);
    }
}
