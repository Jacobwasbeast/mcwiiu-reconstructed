#include "game/ui/FJ_DispenserMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DispenserMenu::FJ_DispenserMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_DispenserMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DispenserMenu(renderNode);
}

unsigned int FJ_DispenserMenu::getType()
{
    return 0x20;
}

void FJ_DispenserMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    FJ_FuiNodeStage* stage = getStage();
    float height = stage->getStageHeight();
    if (height == 360.0 || height == 540.0)
    {
        setX(-right);
        setY(top);
    }
}
