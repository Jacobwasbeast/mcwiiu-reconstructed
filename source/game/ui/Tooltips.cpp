#include "game/ui/Tooltips.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "platform/wiiu/CProfile.hpp"

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
    static const EControllerActions extraTooltipActions[5] = {
        EControllerActions_ConfirmReady, EControllerActions_Vote,
        EControllerActions_LookBehind, EControllerActions_RestartSection,
        EControllerActions_RestartRace
    };
    const int playerId = player->mUserIndex;

    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
    {
        OverwriteTooltip(playerId, EControllerActions_Crafting,
                         player->mAbilities.mInstabuild ? IDS_TOOLTIPS_CREATIVE : IDS_CONTROLS_CRAFTING);
        if (player->pickBlock(true) != 0)
            OverwriteTooltip(playerId, EControllerActions_PickBlock, IDS_CONTROLS_PICKBLOCK);
        else
            ResetTooltip(playerId, EControllerActions_PickBlock);
    }
    else
    {
        ResetTooltip(playerId, EControllerActions_Crafting);
        ResetTooltip(playerId, EControllerActions_PickBlock);
    }

    if (player->isUnderLiquid(Material::WATER) && !player->isPassenger())
        OverwriteTooltip(playerId, EControllerActions_Jump, static_cast<int>(IDS_TOOLTIPS_SWIMUP));
    else
        ResetTooltip(playerId, EControllerActions_Jump);

    if (player->isSleeping() && level != NULL)
    {
        ResetTooltip(playerId, EControllerActions_Action);
        OverwriteTooltip(playerId, EControllerActions_Use, IDS_TOOLTIPS_WAKEUP);
        return;
    }

    if (player->isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = player->getVehicle();
        if ((vehicle->GetType() & EntityType::MINECART) == EntityType::MINECART ||
            vehicle->GetType() == EntityType::BOAT)
            OverwriteTooltip(playerId, EControllerActions_Sneak, static_cast<int>(IDS_TOOLTIPS_EXIT));
        else
            OverwriteTooltip(playerId, EControllerActions_Sneak, static_cast<int>(IDS_TOOLTIPS_DISMOUNT));
    }
    else
    {
        ResetTooltip(playerId, EControllerActions_Sneak);
    }

    not_null_ptr<ItemInstance> item = player->getMainHandItem();
    if (hitResult == NULL)
    {
        if (!item->isEmpty() && player->isAllowedToUse(item))
        {
            ItemTooltipDataHolder itemData(player, item, level, interactionResult);
            OverwriteTooltip(playerId, EControllerActions_Use,
                             item->getItem()->GetUseTooltip(itemData));
            ResetTooltip(playerId, EControllerActions_Action);
        }
        else
        {
            ResetTooltip(playerId, EControllerActions_Use);
            ResetTooltip(playerId, EControllerActions_Action);
        }
    }
    else if (hitResult->mType == HitResult::MISS)
    {
        if (!item->isEmpty())
        {
            ItemTooltipDataHolder itemData(player, item, level, interactionResult);
            OverwriteTooltip(playerId, EControllerActions_Use,
                             item->getItem()->GetUseTooltip(itemData));
            ResetTooltip(playerId, EControllerActions_Action);
        }
        else
        {
            ResetTooltip(playerId, EControllerActions_Use);
            ResetTooltip(playerId, EControllerActions_Action);
        }
    }
    else if (hitResult->mType == HitResult::BLOCK)
    {
        BlockPos pos = hitResult->getBlockPos();
        const Direction* face = hitResult->mDirection;
        const BlockState* state = level->getBlockState(pos);
        Block* block = state->getBlock();

        const bool useBlock = !player->isSneaking() && player->isAllowedToUse(block);
        const bool useItemOnBlock = player->isSneaking() && player->isAllowedToUse(block) && item->isEmpty();
        bool canPlace = true;
        if (level->getMaxBuildHeight() - 1 <= pos.y &&
            (face == Direction::UP || level->getMaxBuildHeight() <= pos.y))
            canPlace = false;
        const bool canDestroy = Minecraft::GetInstance()->mGameModes[playerId]->MayDestroy(pos) &&
                                player->isAllowedToMine();

        BlockTooltipDataHolder blockData(player, level, item, state, pos, interactionResult, useBlock,
                                         useItemOnBlock, canDestroy, canPlace);
        int interactTooltip = static_cast<int>(block->GetInteractTooltip(blockData));
        if (interactTooltip == -1 && !item->isEmpty())
        {
            ItemTooltipDataHolder itemData(player, item, level, interactionResult);
            interactTooltip = static_cast<int>(item->getItem()->GetUseTooltip(itemData));
        }
        OverwriteTooltip(playerId, EControllerActions_Use, interactTooltip);
        OverwriteTooltip(playerId, EControllerActions_Action,
                         static_cast<int>(block->GetHitTooltip(blockData)));
    }
    else if (hitResult->mType == HitResult::ENTITY)
    {
        EntityTooltipDataHolder entityData(player, item, player->isAllowedToHurtEntity(hitResult->mEntity) != 0,
                                           player->isAllowedToInteract(hitResult->mEntity));
        int interactTooltip = static_cast<int>(hitResult->mEntity->GetInteractTooltip(entityData));
        if (interactTooltip == -1 && !item->isEmpty() && !hitResult->mEntity->HideUseTooltips())
        {
            ItemTooltipDataHolder itemData(player, item, level, interactionResult);
            interactTooltip = static_cast<int>(item->getItem()->GetUseTooltip(itemData));
        }
        OverwriteTooltip(playerId, EControllerActions_Use, interactTooltip);
        OverwriteTooltip(playerId, EControllerActions_Action,
                         static_cast<int>(hitResult->mEntity->GetHitTooltip(entityData)));
    }

    boost::unordered_map<int, int> extraTooltips(0xb);
    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        const int primaryPad = gProfile->GetPrimaryPad();
        Minecraft* minecraft = Minecraft::GetInstance();
        ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
        if (player->GetGameType() == GameType::LOBBY)
        {
            extraTooltips[0x34] = gameMode->IsPlayerReady(player->mPlayerIndex)
                ? static_cast<int>(IDS_TOOLTIP_NOT_READY) : static_cast<int>(IDS_TOOLTIP_READY);

            std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
            gameMode->GetUniquePlaylist(playlist);
            if (playlist.size() < 2)
            {
                if (!Minecraft::InMiniGame(eMiniGameId_Battle, true) &&
                    minecraft->GetMiniGame()->HasMultipleVariants())
                {
                    if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                    {
                        if (minecraft->GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                            extraTooltips[0x35] = 0x44c3150d;
                    }
                    else if (primaryPad == playerId && CGameNetworkManager::sInstance.IsHost() &&
                             minecraft != NULL &&
                             minecraft->mLocalPlayers[playerId]->GetGameType() == GameType::LOBBY &&
                             minecraft->GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                    {
                        extraTooltips[0x35] = 0x66f20471;
                    }
                }
            }
            else if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
            {
                extraTooltips[0x35] = 0x1a66548e;
            }
            else if (primaryPad == playerId && CGameNetworkManager::sInstance.IsHost() && minecraft != NULL &&
                     minecraft->mLocalPlayers[playerId]->GetGameType() == GameType::LOBBY)
            {
                extraTooltips[0x35] = IDS_MG_SELECT_MAP;
            }
        }

        if (Minecraft::InMiniGame(eMiniGameId_Tumble, false))
        {
            if (item->getItem() == Items::fireworks)
                OverwriteTooltip(playerId, EControllerActions_Use, 0x568a07be);
        }
        else if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            if (player->mThirdPersonView != 2)
                extraTooltips[0x38] = IDS_TOOLTIP_LOOK_BEHIND;

            int activePlayers = 0;
            ClientScoreboard* scoreboard = player->GetLevel()->GetClientScoreboard();
            for (int i = 0; i < 4; ++i)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[i];
                if (localPlayer != NULL &&
                    (!localPlayer->isSpectator() || scoreboard->GetEndZoneEntered(localPlayer->getSUID(), -1)))
                    ++activePlayers;
            }
            if (activePlayers == 1 && !player->isSpectator())
            {
                ClientMasterGameMode* masterGameMode = minecraft->GetClientMasterGameMode();
                if (CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0 &&
                    masterGameMode->IsRoundPlaying() &&
                    !scoreboard->GetEndZoneEntered(player->getSUID(), -1))
                {
                    extraTooltips[0x36] = IDS_TOOLTIP_RESTART_SECTION;
                    extraTooltips[0x37] = static_cast<int>(IDS_TOOLTIP_RESTART_RACE);
                }
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        boost::unordered_map<int, int>::iterator it = extraTooltips.find(extraTooltipActions[i]);
        if (it != extraTooltips.end())
            OverwriteTooltip(playerId, extraTooltipActions[i], it->second);
        else
            ResetTooltip(playerId, extraTooltipActions[i]);
    }

    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle != NULL && (vehicle->GetType() & EntityType::ABSTRACT_HORSE) == EntityType::ABSTRACT_HORSE &&
        !boost::static_pointer_cast<AbstractHorse>(vehicle)->isTamed())
    {
        ResetTooltip(playerId, EControllerActions_Inventory);
    }
    else if (Minecraft::InMiniGame(eMiniGameId_Any, true) &&
             Minecraft::GetInstance()->GetMiniGame()->LockedInventory())
    {
        ResetTooltip(playerId, EControllerActions_Inventory);
    }
    else
    {
        OverwriteTooltip(playerId, EControllerActions_Inventory, static_cast<int>(IDS_CONTROLS_INVENTORY));
    }
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
            !gConsoleUIController.IsSceneInStack(playerId, EUIScene_ConnectingProgress)) {
            gConsoleUIController.SetTooltips((unsigned int)playerId, gTooltips[playerId], false,
                                             false);
            gTooltipsDirty[playerId] = 0;
        }
    }
}
