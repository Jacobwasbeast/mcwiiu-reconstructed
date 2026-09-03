#include "game/ui/FJ_BeaconMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_BeaconMenu::FJ_BeaconMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_BeaconMenu::~FJ_BeaconMenu()
{
}

FJ_BeaconMenu* FJ_BeaconMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BeaconMenu(renderNode);
}

unsigned int FJ_BeaconMenu::getType()
{
    return 0x1c;
}

void FJ_BeaconMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    setX(-static_cast<float>(right) - 176.0f);
    setY(-static_cast<float>(top) - 176.0f);
}
