#include "game/core/GameMode.hpp"

#include "game/core/Minecraft.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

GameMode::GameMode()
    : mMinecraft(Minecraft::GetInstance()), mGameType(NULL)
{
}

GameMode::~GameMode()
{
}

bool GameMode::isSurvival()
{
    return mGameType->isSurvival();
}

bool GameMode::isCreative()
{
    return mGameType->isCreative();
}

bool GameMode::isAdventure()
{
    return mGameType == GameType::ADVENTURE;
}

bool GameMode::isSpectator()
{
    return mGameType == GameType::SPECTATOR;
}

bool GameMode::isLobby()
{
    return mGameType == GameType::LOBBY;
}

bool GameMode::CanRecordStatsAndAchievements()
{
    return mGameType != GameType::SPECTATOR && !isSpectator();
}

bool GameMode::MayDestroy(const BlockPos& pos)
{
    if (isCreative())
        return true;
    if (!mGameType->isBlockPlacingRestricted())
        return true;
    if (isSpectator())
        return false;
    return true;
}

bool GameMode::MayPlace(const BlockPos& pos, BlockItem* item)
{
    if (isSpectator())
        return false;
    return true;
}

bool GameMode::MayUse(const BlockPos& pos, Item* item)
{
    if (isSpectator())
        return false;
    return true;
}

void GameMode::AutoEquipEnabled()
{
    Minecraft::InMiniGame((EMiniGameId)15, true);
}

void GameMode::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                            boost::shared_ptr<ClientboundChatPacket> packet)
{
}

void GameMode::RecordDeath(boost::shared_ptr<Entity> entity)
{
}

void GameMode::RecordItem(not_null_ptr<ItemInstance> item)
{
}

void GameMode::RecordContainer(int type, int slot, int count, int data)
{
}

void GameMode::RecordWalk(int distance, bool sprinting, bool sneaking)
{
}

void GameMode::RecordSwim(int distance, bool sprinting)
{
}

void GameMode::RecordClimb(int distance)
{
}

void GameMode::RecordUseItem(not_null_ptr<ItemInstance> item)
{
}

void GameMode::RecordJump()
{
}

void GameMode::RecordGainPoints(int points)
{
}

void GameMode::ResetTempPoints()
{
}

bool GameMode::RecordSecondaryTargetReached(const std::wstring& name, int target,
                                            GlideRingGenerator::eGlideRingSize size)
{
    return false;
}

bool GameMode::RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third)
{
    return false;
}

bool GameMode::RecordLapCompleted()
{
    return false;
}

void GameMode::RecordThermalEntered()
{
}

bool GameMode::RecordProgress(double first, double second, double third, double fourth)
{
    return false;
}

void GameMode::RecordSpawnedAtCheckpoint(int checkpoint)
{
}

void GameMode::RecordBlockPlaced(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordBlockDestroyed(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordBlockPresent(const BlockPos& pos, int blockId)
{
}

void GameMode::RecordLayerTime()
{
}

void GameMode::RecordSnowballHit()
{
}

void GameMode::RecordHitBySnowball()
{
}

void GameMode::RecordLanding(const BlockPos& pos)
{
}

void GameMode::OnMouseDown()
{
}

void GameMode::RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients,
                                              Inventory* inventory,
                                              not_null_ptr<ItemInstance> result,
                                              bool removeResult)
{
}

void GameMode::RemoveRecipeItemsFromInventory(Recipe::INGREDIENTS_REQUIRED& ingredients,
                                              boost::shared_ptr<Inventory> inventory,
                                              not_null_ptr<ItemInstance> result,
                                              bool removeResult)
{
    RemoveRecipeItemsFromInventory(ingredients, inventory.get(), result, removeResult);
}

void GameMode::UpdateCraftAchievements(boost::shared_ptr<Player> player,
                                       not_null_ptr<ItemInstance> item, eAward award)
{
}

void GameMode::handleCraftItem(int category, int recipe, boost::shared_ptr<Player> player)
{
}

boost::shared_ptr<ItemInstance> GameMode::handleStackedCraftItem(
    std::vector<std::pair<int, int> >& recipes, int category,
    boost::shared_ptr<Player> player)
{
    return boost::shared_ptr<ItemInstance>();
}

void GameMode::RemoveCustomRecipeItemsFromInventory(int recipe, Inventory* inventory)
{
    if (recipe < 0 || inventory == NULL)
        return;
}

void GameMode::RemoveCustomRecipeItemsFromInventory(int recipe, boost::shared_ptr<Inventory> inventory)
{
    RemoveCustomRecipeItemsFromInventory(recipe, inventory.get());
}

void GameMode::handleCustomCraftItem(std::vector<int>& recipes, boost::shared_ptr<Player> player)
{
}
