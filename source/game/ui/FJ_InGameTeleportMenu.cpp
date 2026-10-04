#include "game/ui/FJ_InGameTeleportMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_InGameTeleportMenu::FJ_InGameTeleportMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_InGameTeleportMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_InGameTeleportMenu(renderNode);
}

unsigned int FJ_InGameTeleportMenu::getType()
{
    return 47;
}

void FJ_InGameTeleportMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const float stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0f && stageHeight != 540.0f)
        return;

    setY(static_cast<float>(top));
}
