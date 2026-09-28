#include "game/ui/FJ_BeaconEffectButton.hpp"

#include <string>

FJ_BeaconEffectButton::FJ_BeaconEffectButton(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
}

void FJ_BeaconEffectButton::ChangeState(int state)
{
    static const char* stateLabels[] = {"Default", "Active", "Focused", "Selected"};
    gotoAndStop(std::string(stateLabels[state]));
    m_state = state;
}

void FJ_BeaconEffectButton::SetIcon(int icon)
{
    FJ_FuiNode* iconNode = getChildByName(std::string("Icon"));
    FJ_FuiNode* iconMovieClip = FJ_FuiNode::asMovieClip(iconNode);
    FJ_FuiNode* tickNode = getChildByName(std::string("BeaconEffectTick"));
    FJ_FuiNode* tickMovieClip = FJ_FuiNode::asMovieClip(tickNode);

    if (icon < 1)
    {
        iconMovieClip->gotoAndStop(1);
        tickMovieClip->setVisible(true);
        return;
    }

    const int iconFrames[] = {-1, 12, 9, 2, 14, 11, 5, 3, 6, 13, 16, 7, 8, 1, 0, 4};
    iconMovieClip->gotoAndStop(iconFrames[icon] + 2);
    tickMovieClip->setVisible(false);
}


