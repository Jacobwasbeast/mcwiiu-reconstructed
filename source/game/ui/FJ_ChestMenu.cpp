#include "game/ui/FJ_ChestMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_ChestMenu::FJ_ChestMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_ChestMenu::~FJ_ChestMenu()
{
}

FJ_AbstractContainerDocument* FJ_ChestMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_ChestMenu(renderNode);
}

unsigned int FJ_ChestMenu::getType()
{
    return 31;
}

void FJ_ChestMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    FJ_FuiNodeStage* stage = getStage();
    double height = stage->getStageHeight();
    if (height == 360.0 || height == 540.0)
    {
        setX(-static_cast<float>(right) - 176.0f);
        setY(-static_cast<float>(top) - 176.0f);
    }
}
