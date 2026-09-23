#include "game/ui/Tooltips.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

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
    int actionIndex = (int)action - 0x20;
    if (gTooltips[playerId][actionIndex] == tooltip)
        return;

    gTooltips[playerId][actionIndex] = tooltip;
    gTooltipsDirty[playerId] = 1;
}

void Tooltips::ResetTooltip(int playerId, EControllerActions action)
{
    int actionIndex = (int)action - 0x20;
    if (gTooltips[playerId][actionIndex] != -1) {
        gTooltips[playerId][actionIndex] = -1;
        gTooltipsDirty[playerId] = 1;
    }
}

void Tooltips::SetInteractTooltips(boost::shared_ptr<MultiplayerLocalPlayer> player,
                                   MultiPlayerLevel* level, const HitResult* hitResult,
                                   InteractionResult::EInteractionResult interactionResult)
{
    if (!player)
        return;

    const int playerId = player->mUserIndex;

    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
        OverwriteTooltip(playerId, static_cast<EControllerActions>(0x28),
                         player->mAbilities.mInstabuild ? 0x43f3dee6 : 0x4ef5d075);
    else
        ResetTooltip(playerId, static_cast<EControllerActions>(0x28));

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) ||
        !Minecraft::GetInstance()->GetMiniGame()->LockedInventory())
        OverwriteTooltip(playerId, static_cast<EControllerActions>(0x25), static_cast<int>(0xc2695d1d));
    else
        ResetTooltip(playerId, static_cast<EControllerActions>(0x25));

    ResetTooltip(playerId, (EControllerActions)0x20);
    ResetTooltip(playerId, (EControllerActions)0x21);
    ResetTooltip(playerId, (EControllerActions)0x22);
    ResetTooltip(playerId, (EControllerActions)0x27);
    ResetTooltip(playerId, (EControllerActions)0x2f);

    (void)level;
    (void)hitResult;
    (void)interactionResult;
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
