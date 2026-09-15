#include "game/server/ServerPlayerGameMode.hpp"

#include <cmath>

#include "game/core/GameRulesInstance.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/TridentItem.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/DimensionType.hpp"

ServerPlayerGameMode::ServerPlayerGameMode(Level* level, MasterGameMode* masterGameMode)
    : GameMode(), mLevel(level), mPlayer(NULL), mDestroying(false), mDestroyProgress(0),
      mDestroyPos(), mLastSentState(0), mIsDestroying(false), mPreviousDestroyPos(),
      mLastSentState2(0), mGameModeTicks(0), mGameRules(NULL),
      mMasterGameMode(masterGameMode), mDeaths(0), mKills(0), mUnknown58(0),
      mLastCheckpoint(0)
{
}

ServerPlayerGameMode::~ServerPlayerGameMode()
{
}

void ServerPlayerGameMode::setLevel(ServerLevel* level) { mLevel = level; }
void ServerPlayerGameMode::updateGameMode(const GameType* gameType)
{
    if (mGameType == GameType::NOT_SET)
        SetGameType(gameType);
}
void ServerPlayerGameMode::setGameRules(GameRulesInstance* gameRules) { mGameRules = gameRules; }
void ServerPlayerGameMode::nullifyGameRules() { mGameRules = NULL; }
void ServerPlayerGameMode::setLastCheckpoint(int checkpoint) { mLastCheckpoint = checkpoint; }
unsigned int ServerPlayerGameMode::getLastCheckpoint() { return mLastCheckpoint; }
void ServerPlayerGameMode::RestoreFromOld(ServerPlayerGameMode* oldMode)
{
    SetDeaths(oldMode->GetDeaths());
    setLastCheckpoint(oldMode->getLastCheckpoint());
}
boost::shared_ptr<ServerPlayer> ServerPlayerGameMode::getServerPlayer() const
{
    return boost::static_pointer_cast<ServerPlayer>(mPlayer->shared_from_this());
}
int ServerPlayerGameMode::superDestroyBlock(const BlockPos& pos)
{
    mPreviousDestroyPos = pos;
    boost::shared_ptr<ServerPlayer> serverPlayer = getServerPlayer();
    if (!serverPlayer || mLevel == 0)
        return 0;

    boost::shared_ptr<Player> player = serverPlayer;
    not_null_ptr<ItemInstance> item = player->mInventory->getSelected();
    const BlockState* state = mLevel->getBlockState(pos);
    Block* block = state->getBlock();

    int result = block->playerWillDestroy(mLevel, pos, state, player) ? 1 : 0;
    if (result == 0 && player->canDestroy(state))
    {
        boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(pos);
        block->playerDestroy(mLevel, player, pos, state, blockEntity, item);
        result = 1;
    }
    return result;
}
int ServerPlayerGameMode::destroyBlock(const BlockPos& pos)
{
    boost::shared_ptr<Player> player = mPlayer;

    if (isCreative())
    {
        not_null_ptr<ItemInstance> item = player->getMainHandItem();
        Item* heldItem = item->getItem();
        if (dynamic_cast<SwordItem*>(heldItem) != NULL
            || dynamic_cast<TridentItem*>(heldItem) != NULL)
            return 0;
    }

    if (!MayDestroy(pos))
        return 0;

    const BlockState* state = mLevel->getBlockState(pos);
    Block* block = state->getBlock();
    GetGameType();

    if (mGameType->isBlockPlacingRestricted())
    {
        if (mGameType == GameType::SPECTATOR)
        {
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            if (miniGame == NULL || !miniGame->HasSpectatorParticipation())
                return 0;
        }
        else if (!player->mayBuild())
        {
            not_null_ptr<ItemInstance> item = player->getMainHandItem();
            if (!item->isEmpty())
            {
                if (!item->hasAdventureModeBreakTagForBlock(block))
                    return 0;
            }
            else if (!block->mMaterial->isDestroyedByHand())
                return 0;
        }
    }

    boost::shared_ptr<ServerPlayer> serverPlayer = getServerPlayer();
    mLevel->levelEvent(serverPlayer, 2001, pos, Block::getIdWithData(state));

    int result = superDestroyBlock(pos);

    static_cast<ServerLevel*>(mLevel)->runQueuedSendBlockUpdates();

    int dimensionId = mLevel->mDimension->getType()->getId();
    gMinecraftServer->getPlayers()->prioritiseBlockChanges(pos, dimensionId);

    RecordBlockDestroyed(pos, Block::getId(block));

    if (isCreative())
    {
        serverPlayer->mConnection->send(
            boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket(mLevel, pos)));
    }
    else
    {
        not_null_ptr<ItemInstance> item = player->getMainHandItem();
        not_null_ptr<ItemInstance> toolCopy;
        bool hasCopy = false;
        if (!item->isEmpty())
        {
            toolCopy = item->copy();
            hasCopy = true;
        }

        bool canDestroy = player->canDestroy(state);
        if (!item->isEmpty())
            item->mineBlock(mLevel, state, pos, player);

        if (result != 0 && canDestroy)
        {
            boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(pos);
            if (hasCopy)
                block->playerDestroy(mLevel, player, pos, state, blockEntity, toolCopy);
        }
    }

    return result;
}
void ServerPlayerGameMode::stopDestroyBlock(const BlockPos& pos)
{
    if (pos.equals(mDestroyPos))
    {
        const BlockState* state = mLevel->getBlockState(pos);
        if (state->getMaterial() != Material::AIR)
        {
            mDestroying = false;
            mLevel->destroyBlockProgress(mPlayer->getId(), pos, -1);
            destroyBlock(pos);
        }
    }
}
void ServerPlayerGameMode::abortDestroyBlock()
{
    mDestroying = false;
    mLevel->destroyBlockProgress(mPlayer->getId(), mDestroyPos, -1);
}
bool ServerPlayerGameMode::useItemOn(boost::shared_ptr<Player> player, Level* level,
                                     not_null_ptr<ItemInstance> item,
                                     InteractionHand::EInteractionHand hand, const BlockPos& pos,
                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                     bool unknown, bool* result)
{
    mPreviousDestroyPos = pos;
    return false;
}
bool ServerPlayerGameMode::startDestroyBlock(const BlockPos& pos, const Direction* direction)
{
    mDestroyPos = pos; mDestroying = true; return true;
}
InteractionResult::EInteractionResult ServerPlayerGameMode::useItem(boost::shared_ptr<Player> player,
    Level* level, InteractionHand::EInteractionHand hand, bool simulate)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame == NULL || !miniGame->ArePowerupsActive())
            return InteractionResult::eInteractionResult_Fail;

        PowerupItems::UsePowerupItem(player, Item::getId(item->getItem()));
        item->shrink(1);
        if (item->isEmpty())
        {
            player->setItemInHand(hand, ItemInstance::EMPTY);
            player->mInventory->OnRemove(player->mInventory->m_selected);
        }
        return InteractionResult::eInteractionResult_Success;
    }

    if (!player->isAllowedToUse(item))
        return InteractionResult::eInteractionResult_Pass;

    if (isSpectator() || mGameType == GameType::LOBBY)
        return InteractionResult::eInteractionResult_Pass;

    int savedCount = item->getCount();
    int savedAux = item->getAuxValue();

    InteractionResultHolder holder = item->getItem()->use(level, player, hand);
    InteractionResult::EInteractionResult result = holder.getResult();
    not_null_ptr<ItemInstance> resultItem = holder.getObject();

    if (result == InteractionResult::eInteractionResult_Success)
        RecordUseItem(resultItem);

    if (resultItem == item && item->getCount() == savedCount && item->getUseDuration() < 1
        && item->getAuxValue() == savedAux)
        return result;

    if (result == InteractionResult::eInteractionResult_Fail && item->getUseDuration() > 0
        && !player->isUsingItem())
        return result;

    player->setItemInHand(hand, resultItem);

    if (isCreative() && !resultItem->getItem()->IsArmor() && !isLobby())
    {
        resultItem->setCount(savedCount);
        if (resultItem->isDamageableItem())
            resultItem->setAuxValue(savedAux);
    }

    if (resultItem->isEmpty())
    {
        player->setItemInHand(hand, ItemInstance::EMPTY);
        player->mInventory->OnRemove(player->mInventory->m_selected);
    }

    if (!player->isUsingItem())
    {
        boost::shared_ptr<ServerPlayer> serverPlayer =
            boost::static_pointer_cast<ServerPlayer>(player);
        serverPlayer->refreshContainer(serverPlayer->mContainerMenu);
    }

    return result;
}
void ServerPlayerGameMode::SetGameType(const GameType* gameType)
{
    mGameType = gameType;

    gameType->updatePlayerAbilities(&mPlayer->mAbilities);
    mPlayer->onUpdateAbilities();
    mPlayer->UpdateSize();

    PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->getPlayers() : NULL;

    boost::shared_ptr<ClientboundGameEventPacket> packet(
        new ClientboundGameEventPacket(3, static_cast<float>(gameType->getId()),
                                       mPlayer->mPlayerIndex));
    players->broadcastAll(packet);

    mLevel->updateSleepingPlayerList();
}
const GameType* ServerPlayerGameMode::GetGameType() { return mGameType; }
bool ServerPlayerGameMode::IsNearCentreOf(const AABB* box, ServerPlayer* player, double x, double y, double z)
{
    if (box == NULL || player == NULL)
        return false;

    AABB* scaled = box->scale(x, y, z);
    Vec3* position = Vec3::newTemp(player->mX, player->mY, player->mZ);
    return scaled->insideXZ(position);
}
void ServerPlayerGameMode::tick()
{
    ++mGameModeTicks;

    if (!isSpectator() && Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        RecordLayerTime();

    const float destroyProgressScale = 10.0f;

    if (!mDestroying)
    {
        if (mIsDestroying)
        {
            const BlockState* state = mLevel->getBlockState(mPreviousDestroyPos);
            if (Block::getId(state->getBlock()) == 0)
            {
                const int entityId = mPlayer->getId();
                mLevel->destroyBlockProgress(entityId, mPreviousDestroyPos, -1);
                mIsDestroying = false;
                mLastSentState2 = -1;
                return;
            }

            boost::shared_ptr<ServerPlayer> serverPlayer = getServerPlayer();
            const int elapsedTicks = (mGameModeTicks - mDestroyProgress) + 1;
            const float destroyRate =
                state->getBlock()->getDestroyProgress(state, serverPlayer, mLevel, mDestroyPos);
            const float fractionalProgress = destroyRate * static_cast<float>(elapsedTicks);
            const int sentProgress = static_cast<int>(fractionalProgress * destroyProgressScale);
            if (sentProgress != mLastSentState2)
            {
                const int entityId = mPlayer->getId();
                mLevel->destroyBlockProgress(entityId, mPreviousDestroyPos, sentProgress);
                mLastSentState2 = sentProgress;
            }
        }
    }
    else
    {
        int& destroyStartTick = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3c);
        const BlockState* state = mLevel->getBlockState(mDestroyPos);
        if (Block::getId(state->getBlock()) == 0)
        {
            mDestroying = false;
            return;
        }

        boost::shared_ptr<ServerPlayer> serverPlayer = getServerPlayer();
        const int elapsedTicks = (mGameModeTicks - destroyStartTick) + 1;
        const float destroyRate =
            state->getBlock()->getDestroyProgress(state, serverPlayer, mLevel, mDestroyPos);
        const float fractionalProgress = destroyRate * static_cast<float>(elapsedTicks);
        const int sentProgress = static_cast<int>(fractionalProgress * destroyProgressScale);
        if (sentProgress != mLastSentState2)
        {
            const int entityId = mPlayer->getId();
            mLevel->destroyBlockProgress(entityId, mDestroyPos, sentProgress);
            mLastSentState2 = sentProgress;
        }

        if (fractionalProgress >= 1.0f)
        {
            mDestroying = false;
            destroyBlock(mDestroyPos);
        }
    }
}
void ServerPlayerGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player)
{
    ++mDeaths;
    if (mMasterGameMode != NULL)
        mMasterGameMode->OnDeath(player);
}
unsigned int ServerPlayerGameMode::GetDeaths() { return mDeaths; }
void ServerPlayerGameMode::SetDeaths(int deaths) { mDeaths = deaths; }
unsigned int ServerPlayerGameMode::GetKills() { return 0; }
void ServerPlayerGameMode::SetKills(int) {}

void ServerPlayerGameMode::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                                        boost::shared_ptr<ClientboundChatPacket> packet)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
        masterGameMode->mGameStats.RecordDamage(entity, entry, packet);
}
void ServerPlayerGameMode::RecordDeath(boost::shared_ptr<Entity> entity)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    masterGameMode->mGameStats.RecordDeath(entity);
}
void ServerPlayerGameMode::RecordItem(not_null_ptr<ItemInstance> item)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordItem(player->getSUID(), item);
}
void ServerPlayerGameMode::RecordContainer(int type, int slot, int count, int data)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordContainer(player->getSUID(), type, slot, count, data);
}
void ServerPlayerGameMode::RecordWalk(int distance, bool sprinting, bool sneaking)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordWalk(player->getSUID(), distance, sprinting, sneaking);
}
void ServerPlayerGameMode::RecordSwim(int distance, bool sprinting)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordSwim(player->getSUID(), distance, sprinting);
}
void ServerPlayerGameMode::RecordClimb(int distance)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordClimb(player->getSUID(), distance);
}
void ServerPlayerGameMode::RecordUseItem(not_null_ptr<ItemInstance> item)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordUseItem(player->getSUID(), item);
}
void ServerPlayerGameMode::RecordJump()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordJump(player->getSUID());
}
void ServerPlayerGameMode::RecordGainPoints(int points)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordGainPoints(player->getSUID(), points);
}
void ServerPlayerGameMode::ResetTempPoints()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.ResetTempPoints(player->getSUID());
}
bool ServerPlayerGameMode::RecordSecondaryTargetReached(const std::wstring& name, int target,
                                                        GlideRingGenerator::eGlideRingSize size)
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    return gMinecraftServer->mMasterGameMode->mGameStats.RecordSecondaryTargetReached(
        player->getSUID(), name, target, size) != 0;
}
bool ServerPlayerGameMode::RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return false;

    bool proceed = true;
    if (!third)
    {
        boost::shared_ptr<ServerPlayer> player = getServerPlayer();
        if (!player)
            return false;
        if (masterGameMode->mGameStats.GetLastCheckpointSequenceID(player->getSUID()) != lap - 1)
            proceed = false;
    }

    if (!proceed)
        return false;

    if (second)
        setLastCheckpoint(checkpoint);

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;

    const unsigned int result = masterGameMode->mGameStats.RecordLastCheckpoint(
        player->getSUID(), checkpoint, lap, first);

    if (result != 0 && second && !third)
        player->increaseScore(123);

    return result != 0;
}
bool ServerPlayerGameMode::RecordLapCompleted()
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    gMinecraftServer->mMasterGameMode->mGameStats.RecordLapCompleted(player->getSUID());
    return true;
}
void ServerPlayerGameMode::RecordThermalEntered()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordThermalEntered(player->getSUID());
}
bool ServerPlayerGameMode::RecordProgress(double first, double second, double third, double fourth)
{
    if (gMinecraftServer == NULL || isSpectator())
        return false;
    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return false;
    gMinecraftServer->mMasterGameMode->mGameStats.RecordProgress(
        player->getSUID(), first, second, third, fourth);
    return true;
}
void ServerPlayerGameMode::RecordSpawnedAtCheckpoint(int checkpoint)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordSpawnedAtCheckpoint(player->getSUID(), checkpoint);
}
void ServerPlayerGameMode::RecordBlockPlaced(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    masterGameMode->mGameStats.RecordBlockPlaced(player->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordBlockDestroyed(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    masterGameMode->mGameStats.RecordBlockDestroyed(player->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordBlockPresent(const BlockPos& pos, int blockId)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    masterGameMode->mGameStats.RecordBlockPresent(player->getSUID(), pos, blockId);
}
void ServerPlayerGameMode::RecordLayerTime()
{
    if (isLobby())
        return;

    unsigned long long& layerTimeMs =
        *reinterpret_cast<unsigned long long*>(reinterpret_cast<char*>(this) + 0x50);
    const unsigned long long now = System::processTimeInMilliSecs();
    const unsigned long long elapsedMs = now - layerTimeMs;
    const int elapsedSeconds =
        static_cast<int>(std::floor(static_cast<double>(elapsedMs) / 1000.0));

    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL)
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    const int layer = masterGameMode->GetLayer(player->mY);
    if (elapsedSeconds <= 1 || layer == -1)
        return;

    AABB* layerAabb = masterGameMode->GetLayerAABB(layer);
    const bool nearCentre = IsNearCentreOf(layerAabb, player.get(), 0.5, 1.875, player->mY);
    layerTimeMs = System::processTimeInMilliSecs();
    masterGameMode->mGameStats.RecordTimeOnLayer(player->getSUID(), layer, nearCentre);
}
void ServerPlayerGameMode::RecordSnowballHit()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordSnowballDirectHit(player->getSUID());
}
void ServerPlayerGameMode::RecordHitBySnowball()
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode == NULL || isSpectator())
        return;

    boost::shared_ptr<ServerPlayer> player = getServerPlayer();
    if (!player)
        return;

    masterGameMode->mGameStats.RecordHitBySnowball(player->getSUID());
}
void ServerPlayerGameMode::RecordLanding(const BlockPos& pos)
{
    MasterGameMode* masterGameMode = gMinecraftServer->mMasterGameMode;
    if (masterGameMode != NULL && !isSpectator())
    {
        double y = static_cast<double>(pos.y);
        int layer = masterGameMode->GetLayer(y);
        if (layer == static_cast<int>(masterGameMode->GetLayerCount()) - 1 &&
            masterGameMode->IsOnGeneratedLayer(layer, y))
        {
            masterGameMode->mGameStats.RecordLanding(mPlayer->getSUID());
        }
    }
}
const MiniGameDef* ServerPlayerGameMode::GetMiniGame() { return NULL; }
boost::shared_ptr<Player> ServerPlayerGameMode::GetPlayer() { return boost::static_pointer_cast<Player>(mPlayer->shared_from_this()); }
Level* ServerPlayerGameMode::GetLevel() { return mLevel; }
