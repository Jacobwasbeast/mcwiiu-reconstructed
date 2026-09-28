#include "game/ui/FJ_InGameInfoMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_InGameInfoMenu::FJ_InGameInfoMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode) {
}

FJ_InGameInfoMenu::~FJ_InGameInfoMenu() {
}

FJ_Document* FJ_InGameInfoMenu::Create(fuiRenderNode* renderNode) {
    return new FJ_InGameInfoMenu(renderNode);
}

unsigned int FJ_InGameInfoMenu::getType() {
    return 46;
}

void FJ_InGameInfoMenu::SetSafeZone(int top, int bottom, int left, int right) {
    FJ_Document::SetSafeZone(top, bottom, left, right);
    float height = getStage()->getStageHeight();
    if (height == 360.0 || height == 540.0) {
        setY(top);
    }
}
