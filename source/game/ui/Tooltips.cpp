#include "game/ui/Tooltips.hpp"

#include "game/core/ConsoleUIController.hpp"

namespace
{
int gTooltips[4][25];
unsigned char gTooltipsDirty[4];
}

void Tooltips::staticCtor()
{
    int playerId;
    int action;

    for (playerId = 0; playerId < 4; ++playerId) {
        gTooltipsDirty[playerId] = 1;
        for (action = 0; action < 25; ++action)
            gTooltips[playerId][action] = -1;
    }
}

void Tooltips::OverwriteTooltip(int playerId, EControllerActions action, int tooltip)
{
    int actionIndex = (int)action;
    if (gTooltips[playerId][actionIndex] == tooltip)
        return;

    gTooltips[playerId][actionIndex] = tooltip;
    gTooltipsDirty[playerId] = 1;
}

void Tooltips::ResetTooltip(int playerId, EControllerActions action)
{
    int actionIndex = (int)action;
    if (gTooltips[playerId][actionIndex] != -1) {
        gTooltips[playerId][actionIndex] = -1;
        gTooltipsDirty[playerId] = 1;
    }
}

void Tooltips::SetInteractTooltips(boost::shared_ptr<MultiplayerLocalPlayer> player,
                                   MultiPlayerLevel* level, const HitResult* hitResult,
                                   InteractionResult::EInteractionResult interactionResult)
{
    int playerId;

    if (!player)
        return;

    playerId = 0;
    ResetTooltip(playerId, (EControllerActions)0x20);
    ResetTooltip(playerId, (EControllerActions)0x21);
    ResetTooltip(playerId, (EControllerActions)0x22);
    ResetTooltip(playerId, (EControllerActions)0x25);
    ResetTooltip(playerId, (EControllerActions)0x27);
    ResetTooltip(playerId, (EControllerActions)0x28);
    ResetTooltip(playerId, (EControllerActions)0x2f);

    if (level == 0 && hitResult == 0 &&
        interactionResult == InteractionResult::eInteractionResult_Fail)
        return;
}

void Tooltips::ResetAllTooltips(int playerId)
{
    int action;
    gTooltipsDirty[playerId] = 1;
    for (action = 0; action < 25; ++action)
        gTooltips[playerId][action] = -1;
}

void Tooltips::ForceTooltipReload(int playerId)
{
    if (playerId == 0xff) {
        gTooltipsDirty[0] = 1;
        gTooltipsDirty[1] = 1;
        gTooltipsDirty[2] = 1;
        gTooltipsDirty[3] = 1;
        return;
    }

    gTooltipsDirty[playerId] = 1;
}

void Tooltips::ForceAllPlayerTooltipReset()
{
    ResetAllTooltips(0);
    ResetAllTooltips(1);
    ResetAllTooltips(2);
    ResetAllTooltips(3);
}

void Tooltips::DisplayTooltips(int playerId)
{
    if (gTooltipsDirty[playerId] != 0) {
        if (!gConsoleUIController.GetMenuDisplayed(playerId) &&
            !gConsoleUIController.IsSceneInStack(playerId, (EUIScene)0x1f)) {
            gConsoleUIController.SetTooltips((unsigned int)playerId, gTooltips[playerId], false,
                                             false);
            gTooltipsDirty[playerId] = 0;
        }
    }
}
