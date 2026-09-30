#include "game/ui/FJ_AnvilMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_AnvilMenu::FJ_AnvilMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
    SetCostLabel(L"UserTab", false);
    ShowRedCross(false);
}

void FJ_AnvilMenu::SetCostLabel(const std::wstring& label, bool safe)
{
    FJ_FuiNode* green = getChildByName("MainPanel.costLabelGreen");
    FJ_FuiNode* red = getChildByName("MainPanel.costLabelRed");
    const bool present = !label.empty();
    if (green != 0)
        green->setVisible(present && safe);
    if (red != 0)
        red->setVisible(present && !safe);
}

void FJ_AnvilMenu::ShowRedCross(bool show)
{
    FJ_FuiNode* cross = getChildByName("MainPanel.AnvilCross");
    if (cross != 0)
        cross->setVisible(show);
}

FJ_AnvilMenu* FJ_AnvilMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AnvilMenu(renderNode);
}

unsigned int FJ_AnvilMenu::getType()
{
    return 0x1b;
}

void FJ_AnvilMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    const float stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;
    setX(-right);
    setY(top);
}
