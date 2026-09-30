#include "game/ui/FJ_BrewingStandMenu.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_BrewingStandMenu::FJ_BrewingStandMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_BrewingStandMenu* FJ_BrewingStandMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingStandMenu(renderNode);
}

unsigned int FJ_BrewingStandMenu::getType()
{
    return 0x1d;
}

void FJ_BrewingStandMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL) return;
    const float height = stage->getStageHeight();
    if (height != 360.0 && height != 540.0) return;
    setX(static_cast<float>(-right));
    setY(static_cast<float>(bottom));
}
