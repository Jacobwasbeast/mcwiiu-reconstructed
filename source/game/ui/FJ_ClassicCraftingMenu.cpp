#include "game/ui/FJ_ClassicCraftingMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_ClassicCraftingMenu::FJ_ClassicCraftingMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_ClassicCraftingMenu::~FJ_ClassicCraftingMenu()
{
}

FJ_ClassicCraftingMenu* FJ_ClassicCraftingMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_ClassicCraftingMenu(renderNode);
}

unsigned int FJ_ClassicCraftingMenu::getType()
{
    return 0x1e;
}

void FJ_ClassicCraftingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    float stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0f && stageHeight != 540.0f)
        return;

    setX(-right);
    setY(top);
}
