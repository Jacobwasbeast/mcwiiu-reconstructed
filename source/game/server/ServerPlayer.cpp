#include "game/server/ServerPlayer.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/core/Stat.hpp"
#include "game/item/Inventory.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ClientboundContainerSetDataPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundRemoveEntitiesPacket.hpp"
#include "game/network/ClientboundRemoveMobEffectPacket.hpp"
#include "game/network/ClientboundUpdateMobEffectPacket.hpp"
#include "game/network/ClientboundContainerClosePacket.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ClientboundEntityEventPacket.hpp"
#include "game/network/ClientboundGameEventPacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/network/ClientboundPlayerCombatPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Dimension.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/FireworksMenu.hpp"
#include "game/ui/HorseInventoryMenu.hpp"
#include "game/ui/MerchantMenu.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/LockableContainer.hpp"
#include "game/item/MerchantContainer.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/PlayerEnderChestContainer.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/core/LockCode.hpp"
#include "game/core/Merchant.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/System.hpp"
#include "game/core/GameType.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/core/GameRule.hpp"
#include "game/core/GameRulesInstance.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/ModifiableAttributeInstance.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/DataWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/Connection.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/DimensionType.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/item/Item.hpp"
#include "game/core/GenericStats.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <set>
#include <vector>

namespace
{
class ServerPlayerDataWalker : public DataWalker
{
public:
    CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version)
    {
        const std::wstring rootVehicle(L"RootVehicle");
        if (tag->contains(rootVehicle, 10))
        {
            CompoundTag* root = tag->getCompound(rootVehicle);
            const std::wstring entity(L"Entity");
            if (root->contains(entity, 10))
            {
                root->put(entity, fixer->update(DataFixTypes::eDataFixTypes_Entity,
                                                root->getCompound(entity), version));
            }
        }
        return tag;
    }
};

static void sendClientChatPacket(ServerPlayer* player, const std::wstring& text,
                                 ClientboundChatPacket::EChatPacketMessage message, int entityType)
{
    if (player == NULL || player->mConnection == NULL)
        return;
    boost::shared_ptr<ClientboundChatPacket> packet(new ClientboundChatPacket(text, message, entityType));
    player->mConnection->send(boost::static_pointer_cast<Packet>(packet));
}

static void broadcastClientChatPacket(MinecraftServer* server, const std::wstring& text,
                                      ClientboundChatPacket::EChatPacketMessage message)
{
    if (server == NULL)
        return;
    PlayerList* playerList = server->getPlayers();
    if (playerList == NULL)
        return;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
    if (players == NULL)
        return;
    for (size_t i = 0; i < players->size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = (*players)[i];
        if (player == NULL)
            continue;
        sendClientChatPacket(player.get(), text, message, 0x14);
    }
}

static void setPvpStateFromDamageSource(ServerPlayer* player, const DamageSource* source)
{
    if (player == NULL || source == NULL)
        return;

    if (source == DamageSource::FALL)
        player->mPvpState = 0x19;
    else if (source == DamageSource::IN_FIRE || source == DamageSource::ON_FIRE)
        player->mPvpState = 0x1b;
    else if (source == DamageSource::LAVA)
        player->mPvpState = 0x1a;
    else if (source == DamageSource::DROWN)
        player->mPvpState = 0x1c;
    else if (source == DamageSource::IN_WALL)
        player->mPvpState = 0x1d;
    else if (source->getMsgId() == static_cast<ClientboundChatPacket::EChatPacketMessage>(24) ||
             source == DamageSource::CREATIVE_PLAYER)
        player->mPvpState = 0x1e;
    else if (source == DamageSource::CACTUS)
        player->mPvpState = 0x1f;
    else
    {
        const EntityDamageSource* entitySource = dynamic_cast<const EntityDamageSource*>(source);
        if (entitySource == NULL)
            return;

        boost::shared_ptr<Entity> attacker = entitySource->getEntity();
        if (attacker == NULL)
            return;

        const unsigned long long entityType = attacker->GetType();
        if (entityType == 0x5D540049ULL)
            player->mPvpState = 0x46;
        else if (entityType == 0xD602001ULL)
            player->mPvpState = 0x53;
        else if (entityType == 0x5D541041ULL)
            player->mPvpState = 0x52;
        else if (entityType == 0x5D541841ULL || entityType == 0x8000011ULL)
            player->mPvpState = 0x51;
        else if (entityType == 0x88000012ULL || entityType == 0x2D698003ULL)
            player->mPvpState = 0x50;
        else if (entityType == 0x4D580043ULL || entityType == 0x8000010ULL)
            player->mPvpState = 0x4b;
        else if (entityType == 0x2D680006ULL)
            player->mPvpState = 0x4a;
        else if (entityType == 0x8A400000ULL)
            player->mPvpState = 0x29;
        else if (entityType == 0xC800001ULL || entityType == 0xC800000ULL)
            player->mPvpState = 0x21;
        else if (entityType == 0x5D540441ULL)
            player->mPvpState = 0x26;
        else if (entityType == 0x800000BULL)
            player->mPvpState = 0x22;
        else if (entityType == 0xB000001ULL)
            player->mPvpState = 0x45;
        else if (entityType == 0x2D680005ULL)
            player->mPvpState = 0x49;
        else if (entityType == 0x4D54004BULL)
            player->mPvpState = 0x48;
        else if (entityType == 0x5D544000ULL)
            player->mPvpState = 0x47;
        else if (entityType == 0xD000005ULL)
            player->mPvpState = 0x44;
        else if (entityType == 0x5D540044ULL)
            player->mPvpState = 0x43;
        else if (entityType == 0x5D540047ULL)
            player->mPvpState = 0x42;
        else if (entityType == 0x5D108001ULL)
            player->mPvpState = 0x41;
        else if (entityType == 0x5D540045ULL)
            player->mPvpState = 0x40;
        else if (entityType == 0x5D540043ULL)
            player->mPvpState = 0x3f;
        else if (entityType == 0x5D540046ULL)
            player->mPvpState = 0x3e;
        else if (entityType == 0x5D108000ULL)
            player->mPvpState = 0x28;
        else if (entityType == 0x5D0000C1ULL)
            player->mPvpState = 0x29;
        else if (entityType == 0x5D550041ULL)
            player->mPvpState = 0x2a;
        else if (entityType == 0x5D550044ULL || entityType == 0x5D550043ULL || entityType == 0x5D550040ULL)
            player->mPvpState = 0x25;
        else if (entityType == 0x5D560041ULL)
            player->mPvpState = 0x3d;
        else if (entityType == 0x5D560040ULL)
            player->mPvpState = 0x27;
        else if (entityType == 0x5D540443ULL || entityType == 0x5D540442ULL || entityType == 0x5D540441ULL)
            player->mPvpState = 0x26;
        else if (entityType == 0x5D540041ULL)
            player->mPvpState = 0x23;
        else if (entityType == 0x2D580001ULL)
            player->mPvpState = 0x3c;
        else if (entityType == 0xD580002ULL)
            player->mPvpState = 0x3b;
        else if (entityType == 0xD6C0002ULL)
            player->mPvpState = 0x24;
        else if ((entityType & 0xC800000ULL) == 0xC800000ULL)
        {
            player->mPvpState = 0x20;
            boost::shared_ptr<Player> attackerPlayer = boost::dynamic_pointer_cast<Player>(attacker);
            if (attackerPlayer != NULL && attackerPlayer->mInventory != NULL)
            {
                not_null_ptr<ItemInstance> carried = attackerPlayer->mInventory->getCarried();
                if (carried != NULL && !carried->isEmpty() && carried->getItem() != NULL)
                    Item::getId(carried->getItem());
            }
        }
        else if ((entityType & 0x8A020000ULL) == 0x8A020000ULL)
        {
            boost::shared_ptr<Arrow> arrow = boost::dynamic_pointer_cast<Arrow>(attacker);
            if (arrow != NULL && arrow->mOwner != NULL)
            {
                if ((arrow->mOwner->GetType() & 0xC800000ULL) == 0xC800000ULL)
                    player->mPvpState = 0x21;
                else if (arrow->mOwner->GetType() == 0x5D540441ULL)
                    player->mPvpState = 0x26;
            }
        }
        else
            player->mPvpState = 0x54;
    }
}
}

ServerPlayer::ServerPlayer(MinecraftServer* server, ServerLevel* level, const std::wstring& name, ServerPlayerGameMode* gameMode)
    : Player((Level*)level, name), mConnection(), mServer(server), mGameMode(gameMode),
      mLoadedChunks(), mSentEntityIds(), mTrackedChunks(), mViewDistance(0.0f), mChunkX(0), mChunkZ(0), mChunkRadius(0),
      mChunkIndex(0), mChunkFlags(0), mLastActionTime(0.0f), mDimensionChangeDelay(0), mSpectatorState(0), mExperienceLevel(0),
      mExperienceTotal(0), mLastTick(0), mRemoveEntityCount(0), mRespawnDimension(0), mEntitiesToRemove(0), mLevitationSourceY(NULL), mChangingDimension(0),
      mSpawnInvulnerableTime(0), mPvpState(0), mViewDistanceModifier(0), mIsChangingQuantityOnly(false), latency(0),
      mRespawnState(false), mFallFlying(0), mChangedDimension(0), mTeleportPending(0), mCameraEntity(0), mType(0), mContainerCounter(0)
{
    mDisconnected = false;
}

ServerPlayer::~ServerPlayer()
{
    delete mLevitationSourceY;
}

void ServerPlayer::sendMessage(uint32_t parameterOne, int32_t* parameterTwo, uint32_t parameterThree, std::wstring* parameterFour, uint32_t parameterFive)
{
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundChatPacket(static_cast<ClientboundChatPacket::EChatPacketMessage>(parameterOne), parameterTwo, parameterThree, parameterFour, parameterFive)));
}

uint32_t ServerPlayer::hasPermission(uint32_t parameterOne)
{
    return mServer != NULL && mServer->isCommandBlockEnabled() && parameterOne != 0;
}

BlockPos ServerPlayer::getCommandSenderBlockPosition()
{
    int x = static_cast<int>(std::floor(mX));
    int y = static_cast<int>(std::floor(mY + 0.5));
    int z = static_cast<int>(std::floor(mZ));
    return BlockPos(x, y, z);
}

void ServerPlayer::refreshContainer(AbstractContainerMenu* menu, std::vector<not_null_ptr<ItemInstance> >* items)
{
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetContentPacket(menu->m_containerId, items)));
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket(-1, -1, mInventory->getCarried())));
}

void ServerPlayer::slotChanged(AbstractContainerMenu* menu, int32_t slot, not_null_ptr<ItemInstance> item)
{
    if (dynamic_cast<ResultSlot*>(menu->getSlot(slot)) != NULL || mIsChangingQuantityOnly)
    {
        return;
    }
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket(menu->m_containerId, slot, item)));
}

void ServerPlayer::setContainerData(AbstractContainerMenu* menu, int32_t dataIndex, int32_t value)
{
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetDataPacket(menu->m_containerId, dataIndex, value)));
}

void ServerPlayer::setAllContainerData(AbstractContainerMenu* menu, boost::shared_ptr<Container> container)
{
    for (int index = 0; index < container->getContainerDataCount(); ++index)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetDataPacket(
            menu->m_containerId, index, container->getContainerData(index))));
    }
}

uint64_t ServerPlayer::flagEntitiesToBeRemoved(uint32_t* parameterOne, bool* parameterTwo)
{
    if (parameterOne == NULL || parameterTwo == NULL || mSentEntityIds.empty())
        return 0;
    if (!*parameterTwo)
    {
        *parameterTwo = true;
        std::memset(parameterOne, 0, 0x40);
    }
    for (std::vector<int32_t>::const_iterator it = mSentEntityIds.begin();
         it != mSentEntityIds.end(); ++it)
    {
        const uint32_t id = static_cast<uint32_t>(*it);
        if (id < 0x800)
            parameterOne[id >> 5] |= 0x80000000U >> (id & 0x1f);
    }
    return 0;
}

int32_t ServerPlayer::getFlagIndexForChunk(ChunkPos* parameterOne, int32_t parameterTwo)
{
    if (parameterOne == NULL)
        return 0;
    unsigned long long index = LevelRenderer::getGlobalIndexForChunk(
        parameterOne->x * 16, 0, parameterOne->z * 16, parameterTwo);
    return static_cast<int32_t>(index / 16);
}

void ServerPlayer::disconnect()
{
    ejectPassengers();
    mDisconnected = true;
    if (mField0x6c0 != 0)
    {
        stopSleepInBed(true, false, false);
    }
}

ServerLevel* ServerPlayer::getLevel()
{
    return static_cast<ServerLevel*>(mLevel);
}

void ServerPlayer::doTickA()
{
    PIXBeginNamedEvent(1.035f, "Player::tick");
    Player::tick();
    PIXEndNamedEvent();

    PIXBeginNamedEvent(1.035f, "Update inventory");
    PIXEndNamedEvent();
}

uint64_t ServerPlayer::broadcast(boost::shared_ptr<BlockEntity>* parameterOne, bool parameterTwo)
{
    if (parameterOne == NULL || !*parameterOne)
        return 0;
    boost::shared_ptr<Packet> packet = (*parameterOne)->getUpdatePacket();
    if (!packet || mConnection == NULL)
        return 0;
    if (parameterTwo)
        mConnection->queueSend(packet);
    else
        mConnection->send(packet);
    return 0;
}

void ServerPlayer::doChunkSendingTick(bool parameterOne, bool parameterTwo)
{
    if (mLoadedChunks.empty() || mConnection == NULL || mServer == NULL || mLevel == NULL)
        return;

    bool forceImmediateResend = false;
    ChunkPos closestChunk(0, 0);
    bool foundClosest = false;
    double closestDistance = std::numeric_limits<double>::max();

    for (std::list<ChunkPos>::iterator it = mLoadedChunks.begin(); it != mLoadedChunks.end();)
    {
        const ChunkPos chunkPos = *it;
        if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true))
        {
            MasterGameMode* gameMode = mServer->mMasterGameMode;
            if (mShowOnMaps && gameMode != NULL &&
                !MasterGameMode::IsChunkWithinActiveBounds(gameMode, chunkPos.x, chunkPos.z))
            {
                it = mLoadedChunks.erase(it);
                continue;
            }
        }

        if (!mLevel->isChunkFinalised(chunkPos.x, chunkPos.z))
        {
            ++it;
            continue;
        }

        if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true) && mServer->mMasterGameMode != NULL &&
            mServer->mMasterGameMode->IsChunkWithinLobbyRegionOfRoundLevel(chunkPos.x, chunkPos.z))
        {
            ++it;
            continue;
        }

        if (parameterTwo && Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true) &&
            mServer->mMasterGameMode != NULL && mServer->mMasterGameMode->GetChunkHistory() == 0)
        {
            ++it;
            continue;
        }

        const double distance = chunkPos.distanceToSqr(mX, mZ);
        if (!foundClosest || distance < closestDistance)
        {
            closestChunk = chunkPos;
            closestDistance = distance;
            foundClosest = true;
        }
        ++it;
    }

    if (!foundClosest)
        return;

    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true) && mServer->mMasterGameMode != NULL)
    {
        if (mServer->mMasterGameMode->GetChunkHistory() == 0)
            forceImmediateResend = true;
        else
            parameterOne = true;
    }

    if (!mConnection->isLocal())
    {
        if (!parameterOne)
        {
            INetworkPlayer* sendPlayer = mConnection->getNetworkPlayer();
            if (sendPlayer == NULL ||
                !MinecraftServer::chunkPacketManagement_CanSendTo(sendPlayer))
                return;

            Connection* connection = mConnection->getConnection();
            if (connection != NULL && connection->countDelayedPackets() > 3)
                return;

            INetworkPlayer* hostPlayer = CGameNetworkManager::sInstance.GetHostPlayer();
            if (hostPlayer != NULL && hostPlayer->GetSinglePlayerOutstandingAckCount() > 3)
                return;
        }

        INetworkPlayer* sendPlayer = mConnection->getNetworkPlayer();
        if (sendPlayer != NULL)
            MinecraftServer::chunkPacketManagement_DidSendTo(sendPlayer);
    }

    ServerLevel* serverLevel = mServer->getLevel(mDimension);
    if (serverLevel == NULL)
        return;

    const int flagIndex = getFlagIndexForChunk(&closestChunk, serverLevel->mDimension->getType()->getId());
    INetworkPlayer* networkPlayer = mConnection->getNetworkPlayer();
    if (networkPlayer != NULL &&
        CGameNetworkManager::sInstance.SystemFlagGet(networkPlayer, flagIndex) &&
        !mConnection->isLocal())
    {
        mLoadedChunks.remove(closestChunk);
        return;
    }

    LevelChunk* chunk = serverLevel->getChunk(closestChunk.x, closestChunk.z);
    if (chunk != NULL && chunk->getHasGapsToCheck() && serverLevel->mDimension->isHasSkyLight())
        chunk->recheckGaps(false);

    if (!mConnection->isLocal())
        mLoadedChunks.remove(closestChunk);

    boost::shared_ptr<Packet> packet;
    if (parameterTwo && Minecraft::InMiniGame(static_cast<EMiniGameId>(15), true) &&
        mServer->mMasterGameMode != NULL && mServer->mMasterGameMode->GetChunkHistory() != 0)
    {
        packet.reset(new BlockRegionUpdatePacket(
            closestChunk.x << 4, 0, closestChunk.z << 4, 16, 256, 16, serverLevel, true, NULL));
    }
    else
    {
        packet.reset(new BlockRegionUpdatePacket(
            closestChunk.x << 4, 0, closestChunk.z << 4, 16, 256, 16, serverLevel, false, NULL));
    }

    if (parameterOne || !packet->isAync())
        mConnection->send(packet);
    else
        mConnection->queueSend(packet);

    if (networkPlayer != NULL)
        CGameNetworkManager::sInstance.SystemFlagSet(networkPlayer, flagIndex);

    const int regionMinX = closestChunk.x * 16;
    const int regionMinZ = closestChunk.z * 16;
    std::vector<boost::shared_ptr<BlockEntity> >* blockEntities =
        serverLevel->getBlockEntitiesInRegion(regionMinX, 0, regionMinZ,
                                              regionMinX + 16, 256, regionMinZ + 16);
    if (blockEntities != NULL)
    {
        for (size_t entityIndex = 0; entityIndex < blockEntities->size(); ++entityIndex)
        {
            boost::shared_ptr<BlockEntity> entity = (*blockEntities)[entityIndex];
            const bool queueBlockEntity = !mConnection->isLocal() && !parameterOne;
            broadcast(&entity, queueBlockEntity);
        }
        delete blockEntities;
    }

    if (!parameterTwo && forceImmediateResend)
        doChunkSendingTick(true, true);
}

uint64_t ServerPlayer::doTickB()
{
    processDimensionDelay();
    return 0;
}

void ServerPlayer::doTick(bool parameterOne, bool parameterTwo, bool parameterThree)
{
    mIgnorePortals = parameterThree;

    if (parameterOne)
    {
        PIXBeginNamedEvent(1.035f, "Update frame tick");
        updateFrameTick();
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(1.035f, "doTickA");
    doTickA();
    PIXEndNamedEvent();

    if (parameterOne)
    {
        PIXBeginNamedEvent(1.035f, "doChunkSendingTick");
        doChunkSendingTick(parameterTwo, false);
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(1.035f, "doTickB");
    doTickB();
    PIXEndNamedEvent();

    mIgnorePortals = false;
}

void ServerPlayer::initMenu()
{
    if (mInventoryMenu != NULL)
        mInventoryMenu->addSlotListener(this);
}

uint64_t ServerPlayer::SpectatePlayerChange(uint32_t parameterOne, int32_t parameterTwo)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* allPlayers = gMinecraftServer->getPlayers()->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> > candidates;
    int32_t currentIndex = -1;

    for (size_t index = 0; index < allPlayers->size(); ++index)
    {
        if (index >= mLevel->mPlayers.size() || !mLevel->mPlayers[index])
            continue;
        if (mLevel->mPlayers[index]->isSpectator())
            continue;

        boost::shared_ptr<ServerPlayer> candidate = (*allPlayers)[index];
        candidates.push_back(candidate);
        if (candidate.get() == GetCamera().get())
            currentIndex = static_cast<int32_t>(candidates.size()) - 1;
    }

    boost::shared_ptr<ServerPlayer> target;
    if (!candidates.empty())
    {
        int32_t count = static_cast<int32_t>(candidates.size());
        int32_t newIndex;
        if (currentIndex == -1)
        {
            newIndex = parameterTwo != 0 ? -1 : 0;
        }
        else if (parameterTwo == 0)
        {
            newIndex = currentIndex + 1;
        }
        else
        {
            newIndex = currentIndex - 1;
        }
        newIndex = ((newIndex % count) + count) % count;

        target = candidates[newIndex];
        std::wstring name = target->getName();
        int32_t messageParams[2] = { newIndex + 1, count };
        sendMessage(0x9a, messageParams, 2, &name, 1);
    }

    SetCamera(target);
    return 0;
}

void ServerPlayer::updateClientsCameras()
{
    boost::shared_ptr<Entity> camera = GetCamera();
    GameModePacket::SetCameraData data;
    data.camera = mPlayerIndex;
    data.target = camera ? camera->getId() : -1;
    gMinecraftServer->getPlayers()->broadcastAll(
        GameModePacket::Create(static_cast<GameModePacket::EMessage>(2), data));
}

uint64_t ServerPlayer::notifyPlayerRespawn(boost::shared_ptr<ServerPlayer>* parameterOne, boost::shared_ptr<ServerPlayer>* parameterTwo)
{
    if (mCamera && mCamera->getId() == (*parameterOne)->getId())
    {
        if (!(*parameterTwo)->isSpectator())
        {
            SetCamera(*parameterTwo);
        }
        else
        {
            SpectatePlayerChange(0, 0);
        }
        updateClientsCameras();
    }
    return 0;
}

void ServerPlayer::flushEntitiesToRemove()
{
    if (!mSentEntityIds.empty())
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundRemoveEntitiesPacket(arrayWithLength<int>(mSentEntityIds))));

    mSentEntityIds.clear();
}

uint64_t ServerPlayer::markAttributesDirty()
{
    BaseAttributeMap* attributes = getAttributes();
    std::vector<AttributeInstance*> instances;
    attributes->getAttributes(instances);
    for (std::vector<AttributeInstance*>::iterator it = instances.begin();
         it != instances.end(); ++it)
    {
        ModifiableAttributeInstance* instance =
            dynamic_cast<ModifiableAttributeInstance*>(*it);
        if (instance != NULL)
            instance->setDirty();
    }
    return 0;
}

void ServerPlayer::refreshContainer(AbstractContainerMenu* parameterOne)
{
    AbstractContainerMenu::ItemList* items = parameterOne->getItems();
    refreshContainer(parameterOne, items);
    delete items;
}

void ServerPlayer::resetSentInfo()
{
}

int32_t ServerPlayer::getFlagIndexForChunk(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree)
{
    unsigned long long index = LevelRenderer::getGlobalIndexForChunk(
        parameterOne * 16, 0, parameterTwo * 16, parameterThree);
    return static_cast<int32_t>(index / 16);
}

void ServerPlayer::resetChunkFlagsOutsideArea(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree, int32_t parameterFour, int32_t parameterFive)
{
    const int width = parameterFour - parameterTwo;
    const int height = parameterThree - parameterOne;
    std::vector<unsigned char> snapshot(static_cast<size_t>(width) * static_cast<size_t>(height));

    for (int x = parameterOne; x < parameterThree; ++x)
    {
        for (int z = parameterTwo; z < parameterFour; ++z)
        {
            const int flagIndex = getFlagIndexForChunk(x, z, parameterFive);
            snapshot[(x - parameterOne) * width + (z - parameterTwo)] =
                static_cast<unsigned char>(CGameNetworkManager::sInstance.SystemFlagGet(mConnection->getNetworkPlayer(), flagIndex));
        }
    }

    CGameNetworkManager::sInstance.SystemFlagClearAll(mConnection->getNetworkPlayer());

    for (int x = parameterOne; x < parameterThree; ++x)
    {
        for (int z = parameterTwo; z < parameterFour; ++z)
        {
            const int flagIndex = getFlagIndexForChunk(x, z, parameterFive);
            if (snapshot[(x - parameterOne) * width + (z - parameterTwo)] != 0)
                CGameNetworkManager::sInstance.SystemFlagSet(mConnection->getNetworkPlayer(), flagIndex);
        }
    }
}

void ServerPlayer::readAdditionalSaveData(CompoundTag* parameterOne)
{
    Player::readAdditionalSaveData(parameterOne);

    if (parameterOne->contains(std::wstring(L"playerGameType"), 99))
    {
        if (mServer->getForceGameType())
            mGameMode->SetGameType(mServer->getDefaultGameType());
        else
            mGameMode->SetGameType(GameType::byId(parameterOne->getInt(std::wstring(L"playerGameType"))));
    }

    if (parameterOne->contains(std::wstring(L"GameRules")) && mGameMode->mGameRules != NULL)
    {
        ByteArrayInputStream bytes(parameterOne->getByteArray(std::wstring(L"GameRules")));
        DataInputStream input(&bytes);
        mGameMode->mGameRules->read(&input);
        input.close();
        bytes.close();
    }
}

void ServerPlayer::addWalkers(DataFixerUpper* parameterOne)
{
    parameterOne->addWalker(DataFixTypes::eDataFixTypes_Player, new ServerPlayerDataWalker());
}

void ServerPlayer::addAdditonalSaveData(CompoundTag* parameterOne)
{
    Player::addAdditonalSaveData(parameterOne);

    if (mGameMode->mGameRules != NULL)
    {
        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        mGameMode->mGameRules->write(&output);
        parameterOne->putByteArray(std::wstring(L"GameRules"), bytes.toByteArray());
        output.close();
    }

    boost::shared_ptr<Entity> rootVehicle = getRootVehicle();
    boost::shared_ptr<Entity> vehicle = getVehicle();
    if (vehicle != NULL && rootVehicle.get() != this)
    {
        std::set<boost::shared_ptr<Entity> >* passengers = rootVehicle->getIndirectPassengers(0xC800001ULL);
        const size_t passengerCount = passengers->size();
        delete passengers;

        if (passengerCount == 1)
        {
            CompoundTag* rootTag = new CompoundTag();
            CompoundTag* entityTag = new CompoundTag();
            rootVehicle->saveAsPassenger(entityTag);
            rootTag->putString(std::wstring(L"Attach"), vehicle->getUUID());
            rootTag->put(std::wstring(L"Entity"), entityTag);
            parameterOne->put(std::wstring(L"RootVehicle"), rootTag);
        }
    }
}

void ServerPlayer::giveExperienceLevels(int32_t parameterOne)
{
    Player::giveExperienceLevels(parameterOne);
    mExperienceLevel = -1;
}

void ServerPlayer::onEnchantmentPerformed(int32_t parameterOne)
{
    Player::onEnchantmentPerformed(parameterOne);
}

void ServerPlayer::onEnterCombat()
{
    LivingEntity::onEnterCombat();
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundPlayerCombatPacket(getCombatTracker(), ClientboundPlayerCombatPacket::ENTER_COMBAT)));
}

void ServerPlayer::onLeaveCombat()
{
    LivingEntity::onLeaveCombat();
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundPlayerCombatPacket(getCombatTracker(), ClientboundPlayerCombatPacket::END_COMBAT)));
}

void ServerPlayer::ResetSpawnInvulnerableTime()
{
    mSpawnInvulnerableTime = 60;
}

void ServerPlayer::tick()
{
    Player::tick();
    processDimensionDelay();
}

uint64_t ServerPlayer::die(DamageSource* parameterOne)
{
    Player::die(parameterOne);
    return 0;
}

int32_t ServerPlayer::hurt(uint64_t parameterOne, float parameterTwo)
{
    const DamageSource* source = reinterpret_cast<const DamageSource*>(static_cast<uintptr_t>(parameterOne));

    if (isInvulnerableTo(source) ||
        ((!mServer->isPvpAllowed() || Minecraft::InMiniGame(static_cast<EMiniGameId>(1), false)) &&
         mSpawnInvulnerableTime > 0 &&
         source != DamageSource::OUT_OF_WORLD && source != DamageSource::CREATIVE_PLAYER))
        return 0;

    if (source != NULL)
    {
        const EntityDamageSource* entitySource = dynamic_cast<const EntityDamageSource*>(source);
        if (entitySource != NULL)
        {
            boost::shared_ptr<Entity> attacker = entitySource->getEntity();

            if ((attacker->GetType() & 0xC800000ULL) == 0xC800000ULL)
            {
                boost::shared_ptr<Player> attackerPlayer = boost::static_pointer_cast<Player>(attacker);
                if (!canHarmPlayer(attackerPlayer))
                    return 0;
            }

            if (attacker != NULL && (attacker->GetType() & 0x8A020000ULL) == 0x8A020000ULL)
            {
                if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureArrowCanKnockback())
                {
                    boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(attacker);
                    if (arrow->mOwner != NULL && (arrow->mOwner->GetType() & 0xC800000ULL) == 0xC800000ULL)
                    {
                        boost::shared_ptr<Player> ownerPlayer = boost::dynamic_pointer_cast<Player>(arrow->mOwner);
                        if (!canHarmPlayer(ownerPlayer))
                            return 0;
                    }
                }
            }
        }
    }

    if (!Player::hurt(source, parameterTwo))
        return 0;

    setPvpStateFromDamageSource(this, source);
    source->sendDamageIndicatorUpdate(boost::static_pointer_cast<ServerPlayer>(shared_from_this()));
    return 1;
}

void ServerPlayer::isPvpAllowed()
{
    if (mServer != NULL)
        mServer->isPvpAllowed();
}

uint32_t ServerPlayer::canHarmPlayer(boost::shared_ptr<Player> parameterOne)
{
    if (parameterOne == NULL || mServer == NULL || !mServer->isPvpAllowed() ||
        !isAllowedToAttackPlayers())
        return 0;
    return Player::canHarmPlayer(parameterOne);
}

int32_t ServerPlayer::canHarmPlayer(std::wstring parameterOne)
{
    if (mServer == NULL || !mServer->isPvpAllowed())
        return 0;
    return Player::canHarmPlayer(parameterOne);
}

void ServerPlayer::addEffect(MobEffectInstance* parameterOne, const boost::shared_ptr<Entity>& parameterTwo)
{
    if (parameterOne != NULL && parameterOne->getEffect() == MobEffects::LEVITATION)
    {
        if (parameterTwo && parameterTwo->GetType() == 0x0000000008000010ULL)
        {
            if (mLevitationSourceY == NULL)
                mLevitationSourceY = new double(mY);
        }
        else
        {
            delete mLevitationSourceY;
            mLevitationSourceY = NULL;
        }
    }
    boost::shared_ptr<Entity> emptySource;
    LivingEntity::addEffect(parameterOne, emptySource);
}

boost::shared_ptr<Entity> ServerPlayer::changeDimension(int dimension)
{
    if (mChangingDimension != 0)
        return boost::shared_ptr<Entity>();

    mChangingDimension = 1;

    if (isSleeping())
        stopSleepInBed(true, false, false);

    const int oldDimension = mDimension;
    if (oldDimension == 1 && dimension == 1)
        awardStat(GenericStats::winGame(), GenericStats::param_winGame());
    else if (oldDimension == 0 && dimension == 1)
        awardStat(GenericStats::theEnd(), GenericStats::param_theEnd());

    if (oldDimension == 1 && dimension == 1)
    {
        assert(mConnection != NULL);
        INetworkPlayer* networkPlayer = mConnection->getNetworkPlayer();
        const float playerIndex = static_cast<float>(networkPlayer->GetSmallId());
        if (mRespawnState == false)
        {
            mRespawnState = true;
            *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(this) + 0x935) = 1;
            mConnection->send(boost::shared_ptr<Packet>(
                new ClientboundGameEventPacket(4, playerIndex)));

            PlayerList* playerList = mServer->getPlayers();
            std::vector<boost::shared_ptr<ServerPlayer> >* players = playerList->getPlayers();
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
                 it != players->end(); ++it)
            {
                boost::shared_ptr<ServerPlayer> player = *it;
                assert(player != NULL);
                assert(player->mConnection != NULL);
                INetworkPlayer* otherNetworkPlayer = player->mConnection->getNetworkPlayer();
                if (otherNetworkPlayer == NULL || otherNetworkPlayer == networkPlayer)
                    continue;
                if (!networkPlayer->GetSendQueueSizeMessages(otherNetworkPlayer, false))
                    continue;
                if (player->mRespawnState != false)
                    continue;
                player->mRespawnState = true;
                player->mConnection->send(boost::shared_ptr<Packet>(
                    new ClientboundGameEventPacket(4, playerIndex)));
            }
        }
    }

    if (oldDimension == 0 && dimension == 1)
    {
        assert(mConnection != NULL);
        ServerLevel* destination = mServer->getLevel(1);
        BlockPos* spawn = destination->getDimensionSpecificSpawn();
        if (spawn != NULL)
        {
            const double spawnX = static_cast<double>(static_cast<uint32_t>(spawn->x));
            const double spawnY = static_cast<double>(static_cast<uint32_t>(spawn->y));
            const double spawnZ = static_cast<double>(static_cast<uint32_t>(spawn->z));
            mConnection->teleport(spawnX, spawnY, spawnZ,
                                  1.0349999666f, 1.0349999666f, true, true);
        }
    }

    boost::shared_ptr<ServerPlayer> self =
        boost::dynamic_pointer_cast<ServerPlayer>(shared_from_this());
    mServer->getPlayers()->toggleDimension(self, dimension);

    assert(mConnection != NULL);
    mConnection->send(boost::shared_ptr<Packet>(
        new ClientboundLevelEventPacket(0, BlockPos(), 0x408, false)));

    *reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x8f4) = -1.0f;
    *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(this) + 0x8f8) = -1;
    *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(this) + 0x900) = -1;

    return self;
}

uint32_t ServerPlayer::broadcastToPlayer(boost::shared_ptr<ServerPlayer>* parameterOne)
{
    if ((*parameterOne)->isSpectator())
    {
        return GetCamera().get() == this;
    }
    if (isSpectator())
    {
        return 0;
    }
    return Entity::broadcastToPlayer(*parameterOne);
}

uint64_t ServerPlayer::take(boost::shared_ptr<Entity>* parameterOne, int32_t parameterTwo)
{
    LivingEntity::take(*parameterOne, parameterTwo);
    return 0;
}

int32_t ServerPlayer::startSleepInBed(BlockPos* parameterOne, bool parameterTwo)
{
    if (parameterOne == NULL)
        return 0;
    return Player::startSleepInBed(*parameterOne, parameterTwo);
}

uint64_t ServerPlayer::stopSleepInBed(bool parameterOne, bool parameterTwo, bool parameterThree)
{
    if (isSleeping())
    {
        boost::shared_ptr<Entity> self = shared_from_this();
        getLevel()->getTracker()->broadcastAndSend(self,
            boost::shared_ptr<Packet>(new ClientboundAnimatePacket(self, 2)));
    }

    Player::stopSleepInBed(parameterOne, parameterTwo, parameterThree);

    if (mConnection != NULL)
        mConnection->teleport(mX, mY, mZ, mYRot, mXRot, true, true);

    return 0;
}

uint32_t ServerPlayer::startRiding(boost::shared_ptr<Entity>* parameterOne)
{
    boost::shared_ptr<Entity> vehicleBefore = getVehicle();

    if (!Player::startRiding(*parameterOne))
        return 0;

    boost::shared_ptr<Entity> vehicleAfter = getVehicle();
    if (vehicleAfter.get() != vehicleBefore.get() && mConnection != NULL)
        mConnection->teleport(mX, mY, mZ, mYRot, mXRot, true, true);

    return 1;
}

void ServerPlayer::stopRiding()
{
    boost::shared_ptr<Entity> vehicleBefore = getVehicle();
    Player::stopRiding();
    boost::shared_ptr<Entity> vehicleAfter = getVehicle();
    if (vehicleBefore.get() != vehicleAfter.get() && mConnection)
        mConnection->teleport(mX, mY, mZ, mYRot, mXRot, true, true);
}

void ServerPlayer::stopRidingBeforeTeleport()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();
    Player::stopRiding();
    if (vehicle)
        fjImmediateBroadcastPassengerPacket(vehicle);
}

uint8_t ServerPlayer::isChangingDimension()
{
    return mChangingDimension;
}

bool ServerPlayer::isInvulnerableTo(const DamageSource* parameterOne)
{
    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true) &&
        ((parameterOne->isFire() && !Entity::isInLava()) || !TakeGlideCollisionDamage()))
    {
        return true;
    }
    bool result = true;
    if (!Entity::isInvulnerableTo(parameterOne) && !isChangingDimension())
    {
        result = false;
    }
    return result;
}

void ServerPlayer::checkFallDamage(double parameterOne, bool parameterTwo, Block* parameterThree, BlockPos* parameterFour)
{
}

uint64_t ServerPlayer::doCheckFallDamage(double parameterOne, bool parameterTwo)
{
    BlockPos pos(mX, mY - 0.2, mZ);
    Block* block = mLevel->getBlock(pos);
    if (block == Blocks::AIR)
    {
        BlockPos belowPos = pos.below();
        Block* belowBlock = mLevel->getBlock(belowPos);
        if (belowBlock != NULL &&
            (dynamic_cast<FireBlock*>(belowBlock) != NULL ||
             dynamic_cast<WallBlock*>(belowBlock) != NULL ||
             dynamic_cast<FenceBlock*>(belowBlock) != NULL))
        {
            pos = belowPos;
            block = belowBlock;
        }
    }
    LivingEntity::checkFallDamage(parameterOne, parameterTwo, block, pos);
    return 0;
}

void ServerPlayer::nextContainerCounter()
{
    mContainerCounter = mContainerCounter % 100 + 1;
}

void ServerPlayer::openTextEdit(boost::shared_ptr<SignBlockEntity> parameterOne)
{
    boost::shared_ptr<SignBlockEntity> sign = boost::dynamic_pointer_cast<SignBlockEntity>(parameterOne);
    if (sign == NULL)
        return;

    sign->setCreatedPlayer(getUUID());
    sign->setCreatedByHostPlayer(false);

    Minecraft* minecraft = Minecraft::GetInstance();
    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[i];
        if (localPlayer != NULL && localPlayer->getUUID() == getUUID())
        {
            sign->setCreatedByHostPlayer(true);
            break;
        }
    }

    mConnection->send(boost::shared_ptr<Packet>(new ClientboundOpenSignEditorPacket(sign->getBlockPos())));
}

int ServerPlayer::startBlockInteraction(InteractionObject& interaction)
{
    if (mContainerMenu == mInventoryMenu)
    {
        nextContainerCounter();

        InteractionObject::NameData nameData = interaction.GetNetworkNameData();
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
            mContainerCounter, interaction.getTypeId(false), nameData)));

        mContainerMenu = interaction.createMenu(mInventory,
                                                boost::dynamic_pointer_cast<Player>(shared_from_this()));
        mContainerMenu->m_containerId = mContainerCounter;
        mContainerMenu->addSlotListener(this);
    }
    return 1;
}

int ServerPlayer::openFireworks(int32_t x, int32_t y, int32_t z)
{
    if (mContainerMenu == mInventoryMenu)
        return 1;

    if (mContainerMenu == NULL)
        return 1;

    if (dynamic_cast<CraftingMenu*>(mContainerMenu) == NULL)
        return 1;

    closeContainer();
    nextContainerCounter();

    InteractionObject::NameData nameData(std::wstring(L""), std::wstring(L""), true, false);
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
        mContainerCounter, 12, nameData, 9, 0)));

    mContainerMenu = new FireworksMenu(mInventory, mLevel, BlockPos(x, y, z));
    mContainerMenu->m_containerId = mContainerCounter;
    mContainerMenu->addSlotListener(this);
    return 1;
}

int ServerPlayer::openContainer(boost::shared_ptr<Container> container)
{
    if (mContainerMenu != mInventoryMenu)
        return 1;

    boost::shared_ptr<LockableContainer> lockable = boost::dynamic_pointer_cast<LockableContainer>(container);
    if (lockable != NULL && lockable->isLocked() && !canUnlock(lockable->getLockKeyCode()) && !isSpectator())
    {
        boost::shared_ptr<ClientboundChatPacket> chatPacket(new ClientboundChatPacket());
        chatPacket->message = ClientboundChatPacket::eChatPacketMessage_ContainerLocked;
        chatPacket->strings.push_back(container->getDisplayName());
        mConnection->send(chatPacket);
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundSoundPacket(
            SoundEvents::CHEST_LOCKED, SoundSource::eSoundSource_Blocks, mX, mY, mZ, 1.0f, 1.0f)));
        return 0;
    }

    nextContainerCounter();

    boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(container);
    if (interaction != NULL)
    {
        InteractionObject::NameData nameData = interaction->GetNetworkNameData();
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
            mContainerCounter, interaction->getTypeId(false), nameData, container->getContainerSize())));

        mContainerMenu = interaction->createMenu(mInventory,
                                                 boost::static_pointer_cast<Player>(shared_from_this()));
    }
    else
    {
        boost::shared_ptr<PlayerEnderChestContainer> enderChest =
            boost::dynamic_pointer_cast<PlayerEnderChestContainer>(container);
        if (enderChest != NULL)
        {
            InteractionObject::NameData nameData = enderChest->GetNetworkNameData();
            mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
                mContainerCounter, 15, nameData, container->getContainerSize())));
        }
        else
        {
            InteractionObject::NameData nameData(container->getDisplayName(), std::wstring(L""), true, false);
            mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
                mContainerCounter, 0, nameData, container->getContainerSize())));
        }

        mContainerMenu = new ChestMenu(mInventory, container,
                                       boost::static_pointer_cast<Player>(shared_from_this()));
    }

    mContainerMenu->m_containerId = mContainerCounter;
    mContainerMenu->addSlotListener(this);
    return 1;
}

int ServerPlayer::openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int entityId)
{
    if (mContainerMenu != mInventoryMenu)
        return 1;

    nextContainerCounter();

    MerchantMenu* menu = new MerchantMenu(mInventory, merchant, mLevel);
    mContainerMenu = menu;
    mContainerMenu->m_containerId = mContainerCounter;
    mContainerMenu->addSlotListener(this);

    boost::shared_ptr<MerchantContainer> tradeContainer = menu->getTradeContainer();

    InteractionObject::NameData nameData(name.empty() ? std::wstring(L"") : name,
                                         std::wstring(L""), true, false);

    boost::shared_ptr<Entity> entity = mLevel->getEntity(entityId);
    if (entity != NULL)
    {
        nameData = InteractionObject::NameData(name.empty() ? std::wstring(L"") : name,
                                               entity->getOwnerUUID(),
                                               entity->wasNamedOnHost(),
                                               entity->wasNamedByRestrictedPlayer());
    }

    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
        mContainerCounter, 6, nameData, tradeContainer->getContainerSize(), entityId)));

    MerchantRecipeList* offers = merchant->getOffers(boost::dynamic_pointer_cast<Player>(shared_from_this()));
    if (offers != NULL)
    {
        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        output.writeInt(mContainerCounter);
        offers->writeToStream(&output);
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundCustomPayloadPacket(
            std::wstring(L"MC|TrList"), bytes.toByteArray())));
    }

    return 1;
}

int ServerPlayer::openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container)
{
    if (mContainerMenu != mInventoryMenu)
        doCloseContainer();

    nextContainerCounter();

    InteractionObject::NameData nameData(horse->getCustomName(), horse->getOwnerUUID(),
                                         horse->wasNamedOnHost(), horse->wasNamedByRestrictedPlayer());
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket(
        mContainerCounter, 11, nameData, container->getContainerSize(), horse->getId())));

    mContainerMenu = new HorseInventoryMenu(mInventory, container, horse,
                                            boost::dynamic_pointer_cast<Player>(shared_from_this()));
    mContainerMenu->m_containerId = mContainerCounter;
    mContainerMenu->addSlotListener(this);
    return 1;
}

void ServerPlayer::openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand)
{
    if (item->getItem() != Items::writtenBook)
        return;

    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.write(static_cast<unsigned int>(hand));
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundCustomPayloadPacket(
        std::wstring(L"MC|BOpen"), bytes.toByteArray())));
}

void ServerPlayer::doCloseContainer()
{
    mContainerMenu->removed(boost::static_pointer_cast<Player>(shared_from_this()));

    if (mContainerMenu != mInventoryMenu)
        delete mContainerMenu;

    mContainerMenu = mInventoryMenu;
    refreshContainer(mContainerMenu);
}

void ServerPlayer::closeContainer()
{
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerClosePacket(mContainerMenu->m_containerId)));
    doCloseContainer();
}

void ServerPlayer::broadcastCarriedItem()
{
    if (mConnection != NULL && mInventory != NULL && !mIsChangingQuantityOnly)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket(-1, -1, mInventory->getCarried())));
}

void ServerPlayer::setPlayerInput(float parameterOne, float parameterTwo, bool parameterThree, bool parameterFour)
{
    if (mConnection && mConnection->isLocal())
    {
        if (parameterOne >= -1.0f && parameterOne <= 1.0f)
            mXxa = parameterOne;
        if (parameterTwo >= -1.0f && parameterTwo <= 1.0f)
            mZza = parameterTwo;
        mJumping = parameterThree;
        setSneaking(parameterFour);
    }
}

void ServerPlayer::awardStat(Stat* stat, arrayWithLength<unsigned char> parameters)
{
    if (stat != NULL && stat->isAwardedFromServer() && CanRecordStats(stat, parameters))
    {
        int value = *(int*)parameters.data;
        delete[] parameters.data;
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundAwardStatPacket(stat->id, value)));
        return;
    }

    delete[] parameters.data;
}

void ServerPlayer::resetStat(Stat* parameterOne)
{
}

uint8_t ServerPlayer::hasDisconnected()
{
    return mDisconnected;
}

void ServerPlayer::displayClientMessage(int32_t parameterOne, bool parameterTwo)
{
    const std::wstring localizedText(L10N::GetString(parameterOne));
    ClientboundChatPacket::EChatPacketMessage packetMessage =
        ClientboundChatPacket::eChatPacketMessage_Text;
    bool broadcast = false;

    switch (parameterOne)
    {
    case 0x00FE5875: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x54); broadcast = true; break;
    case 0x16972FBF: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x4d); broadcast = true; break;
    case 0x16C81B7F:
        sendClientChatPacket(this, std::wstring(), static_cast<ClientboundChatPacket::EChatPacketMessage>(4), -1);
        return;
    case 0x1E0D17D0: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x48); broadcast = true; break;
    case 0x1EE681BA: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x4e); broadcast = true; break;
    case 0x3221ECE8: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x56); broadcast = true; break;
    case 0x3DCD926B: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5e); broadcast = true; break;
    case 0x3E35B6E8: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x49); broadcast = true; break;
    case 0x56C64197:
        broadcastClientChatPacket(mServer, localizedText, static_cast<ClientboundChatPacket::EChatPacketMessage>(6));
        return;
    case 0x60A3835D: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5f); broadcast = true; break;
    case 0x6E3A65E3: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x45); broadcast = true; break;
    case 0x70EDC806: packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x4a); broadcast = true; break;
    case static_cast<int32_t>(0x82460728): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x47); broadcast = true; break;
    case static_cast<int32_t>(0x8ECD701E): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x4f); broadcast = true; break;
    case static_cast<int32_t>(0x99326383): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x53); broadcast = true; break;
    case static_cast<int32_t>(0x9F8BFFAE): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x46); broadcast = true; break;
    case static_cast<int32_t>(0xA82B5A66): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x4c); broadcast = true; break;
    case static_cast<int32_t>(0xB2145884): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x57); broadcast = true; break;
    case static_cast<int32_t>(0xB399BD15): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x52); broadcast = true; break;
    case static_cast<int32_t>(0xB604AAFF): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x58); broadcast = true; break;
    case static_cast<int32_t>(0xB714E0E2): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5d); broadcast = true; break;
    case static_cast<int32_t>(0xC952E8B9): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5b); broadcast = true; break;
    case static_cast<int32_t>(0xD08616D9):
        sendClientChatPacket(this, std::wstring(), static_cast<ClientboundChatPacket::EChatPacketMessage>(1), -1);
        return;
    case static_cast<int32_t>(0xD3560061): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x44); broadcast = true; break;
    case static_cast<int32_t>(0xD97FDBA2): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x50); broadcast = true; break;
    case static_cast<int32_t>(0xE08B6A47):
        sendClientChatPacket(this, std::wstring(), static_cast<ClientboundChatPacket::EChatPacketMessage>(1), -1);
        return;
    case static_cast<int32_t>(0xEC344E3D): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x55); broadcast = true; break;
    case static_cast<int32_t>(0xEF12CC40): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5c); broadcast = true; break;
    case static_cast<int32_t>(0xF084EC0C):
        sendClientChatPacket(this, std::wstring(), static_cast<ClientboundChatPacket::EChatPacketMessage>(1), -1);
        return;
    case static_cast<int32_t>(0xFAB1DEB6): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x59); broadcast = true; break;
    case static_cast<int32_t>(0xFAB38114):
        sendClientChatPacket(this, localizedText, static_cast<ClientboundChatPacket::EChatPacketMessage>(7),
                             parameterTwo ? 1 : 0);
        return;
    case static_cast<int32_t>(0xFDF65C18): packetMessage = static_cast<ClientboundChatPacket::EChatPacketMessage>(0x5a); broadcast = true; break;
    default:
        return;
    }

    if (broadcast)
        broadcastClientChatPacket(mServer, localizedText, packetMessage);
}

void ServerPlayer::completeUsingItem()
{
    if (mUseItem->isEmpty())
        return;

    if (!isUsingItem())
        return;

    boost::shared_ptr<Entity> self = shared_from_this();
    mConnection->send(boost::shared_ptr<Packet>(new ClientboundEntityEventPacket(self, 9, 0)));

    LivingEntity::completeUsingItem();
}

uint64_t ServerPlayer::restoreFrom(boost::shared_ptr<Player>* parameterOne, bool parameterTwo)
{
    if (parameterOne == NULL || parameterOne->get() == NULL)
        return 0;

    Player* const oldPlayer = parameterOne->get();

    const bool copyInventory = parameterTwo
        || (mLevel != NULL && mLevel->getGameRules()->getBoolean(2))
        || !oldPlayer->isSpectator();

    if (copyInventory)
    {
        if (mInventory != NULL && oldPlayer->mInventory != NULL)
            mInventory->replaceWith(oldPlayer->mInventory);
    }

    if (parameterTwo)
    {
        setHealth(oldPlayer->getHealth());
        mFoodData = oldPlayer->mFoodData;
        mExperienceLevel = oldPlayer->mExperienceLevel;
        mTotalExperience = oldPlayer->mTotalExperience;
        mExperienceProgress = oldPlayer->mExperienceProgress;
        setScore(static_cast<int32_t>(oldPlayer->getScore()));
    }
    else if (copyInventory)
    {
        mExperienceLevel = oldPlayer->mExperienceLevel;
        mTotalExperience = oldPlayer->mTotalExperience;
        mExperienceProgress = oldPlayer->mExperienceProgress;
        setScore(static_cast<int32_t>(oldPlayer->getScore()));
    }

    mEnchantmentSeed = oldPlayer->mEnchantmentSeed;
    mEnderChestInventory = oldPlayer->mEnderChestInventory;

    if (mEntityData != NULL && oldPlayer->mEntityData != NULL)
        mEntityData->set(Entity::DATA_SHARED_FLAGS_ID,
                         oldPlayer->mEntityData->get(Entity::DATA_SHARED_FLAGS_ID), true);

    *reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x8f4) = -1.0f;
    *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(this) + 0x8f8) = -1;
    *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(this) + 0x900) = -1;

    const boost::shared_ptr<ServerPlayer> oldServerPlayer =
        boost::dynamic_pointer_cast<ServerPlayer>(*parameterOne);
    if (oldServerPlayer != NULL)
    {
        mSentEntityIds = oldServerPlayer->mSentEntityIds;
        if (mGameMode != NULL && oldServerPlayer->mGameMode != NULL)
            mGameMode->RestoreFromOld(oldServerPlayer->mGameMode);
    }

    CompoundTag* leftShoulder = oldPlayer->getShoulderEntityLeft();
    if (leftShoulder != NULL)
        setShoulderEntityLeft(leftShoulder->copy());

    CompoundTag* rightShoulder = oldPlayer->getShoulderEntityRight();
    if (rightShoulder != NULL)
        setShoulderEntityRight(rightShoulder->copy());

    return 0;
}

void ServerPlayer::onEffectAdded(MobEffectInstance* parameterOne)
{
    LivingEntity::onEffectAdded(parameterOne);
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundUpdateMobEffectPacket(getId(), parameterOne)));
}

void ServerPlayer::onEffectUpdated(MobEffectInstance* parameterOne, bool parameterTwo)
{
    LivingEntity::onEffectUpdated(parameterOne, parameterTwo);
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundUpdateMobEffectPacket(getId(), parameterOne)));
}

void ServerPlayer::onEffectRemoved(MobEffectInstance* parameterOne)
{
    LivingEntity::onEffectRemoved(parameterOne);
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundRemoveMobEffectPacket(getId(), parameterOne->getEffect())));
}

void ServerPlayer::teleportTo(double parameterOne, double parameterTwo, double parameterThree)
{
    if (mConnection != NULL)
        mConnection->teleport(parameterOne, parameterTwo, parameterThree, mYRot, mXRot, true, false);
}

void ServerPlayer::teleportTo(double parameterOne, double parameterTwo, double parameterThree, float parameterFour, float parameterFive)
{
    if (mConnection != NULL)
        mConnection->teleport(parameterOne, parameterTwo, parameterThree, parameterFive, parameterFour, true, true);
}

uint64_t ServerPlayer::crit(boost::shared_ptr<Entity>* parameterOne)
{
    boost::shared_ptr<Entity> self = shared_from_this();
    getLevel()->getTracker()->broadcastAndSend(self,
        boost::shared_ptr<Packet>(new ClientboundAnimatePacket(*parameterOne, 4)));
    return 0;
}

uint64_t ServerPlayer::magicCrit(boost::shared_ptr<Entity>* parameterOne)
{
    boost::shared_ptr<Entity> self = shared_from_this();
    getLevel()->getTracker()->broadcastAndSend(self,
        boost::shared_ptr<Packet>(new ClientboundAnimatePacket(*parameterOne, 5)));
    return 0;
}

void ServerPlayer::onUpdateAbilities()
{
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundPlayerAbilitiesPacket(&mAbilities, getId())));
}

void ServerPlayer::setGameMode(GameType* parameterOne)
{
    if (mGameMode == NULL || parameterOne == NULL)
        return;
    mGameMode->SetGameType(parameterOne);
    setPlayerGamePrivilege(static_cast<Player::EPlayerGamePrivileges>(9),
                           mGameMode->isCreative());
    if (parameterOne == GameType::SPECTATOR)
        removeEntitiesOnShoulder();
}

bool ServerPlayer::isSpectator()
{
    return mSpectatorState != 0;
}

bool ServerPlayer::isCreative()
{
    const GameType* type = GetGameType();
    return type != NULL && type->isCreative();
}

void ServerPlayer::resetLastActionTime()
{
    mLastTick = System::processTimeInMilliSecs();
}

uint64_t ServerPlayer::sendRemoveEntity(boost::shared_ptr<Entity>* parameterOne)
{
    if (((*parameterOne)->GetType() & 0xC800000ULL) == 0xC800000ULL)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundRemoveEntitiesPacket((*parameterOne)->getId())));
        mSentEntityIds.push_back((*parameterOne)->getId());
    }
    return 0;
}

uint64_t ServerPlayer::cancelRemoveEntity(boost::shared_ptr<Entity>* parameterOne)
{
    if (parameterOne == NULL || !*parameterOne)
        return 0;
    const int32_t id = (*parameterOne)->getId();
    std::vector<int32_t>::iterator it = mSentEntityIds.begin();
    for (; it != mSentEntityIds.end() && *it != id; ++it) {}
    if (it != mSentEntityIds.end())
        mSentEntityIds.erase(it);
    return 0;
}

void ServerPlayer::updateInvisibilityStatus()
{
    LivingEntity::updateInvisibilityStatus();
    getLevel()->getTracker()->updateVisibility(boost::dynamic_pointer_cast<ServerPlayer>(shared_from_this()));
}

void ServerPlayer::processDimensionDelay()
{
    if (mDimensionChangeDelay < 1 || mChangingDimension != 0)
        return;
    --mDimensionChangeDelay;
}

uint64_t ServerPlayer::attack(boost::shared_ptr<Entity> parameterOne)
{
    if (mGameMode != NULL && mGameMode->GetGameType() == GameType::SPECTATOR)
        return 0;
    Player::attack(parameterOne);
    return 0;
}

bool ServerPlayer::swing(InteractionHand::EInteractionHand parameterOne)
{
    bool swung = LivingEntity::swing(parameterOne);
    resetAttackStrengthTicker();
    return swung;
}

void ServerPlayer::hasChangedDimension()
{
    mChangingDimension = 0;
}

void ServerPlayer::startFallFlying()
{
    setSharedFlag(9, true);
}

void ServerPlayer::stopFallFlying()
{
    mFallFlying = 0;
    setSharedFlag(9, false);
}

uint32_t ServerPlayer::getPlayerViewDistanceModifier()
{
    if (mConnection == NULL || mConnection->isLocal())
        return 0;
    INetworkPlayer* networkPlayer = mConnection->getNetworkPlayer();
    if (networkPlayer == NULL)
        return 0;
    uint32_t modifier = static_cast<uint32_t>(networkPlayer->GetTimeSinceLastChunkPacket_ms()) >> 6;
    return modifier > 4 ? 4 : modifier;
}

void ServerPlayer::handleCollectItem(not_null_ptr<ItemInstance> parameterOne)
{
    if (mGameMode != NULL && mGameMode->mGameRules != NULL)
        mGameMode->mGameRules->onCollectItem(parameterOne);
}

void ServerPlayer::SetCamera(boost::shared_ptr<Entity> parameterOne)
{
    Player::SetCamera(parameterOne);
    if (isSpectator() && IsSpectatingOtherPlayer() && mCamera)
    {
        teleportTo(mCamera->mX, mCamera->mY, mCamera->mZ, mCamera->mXRot, mCamera->mYRot);
    }
}

const GameType* ServerPlayer::GetGameType()
{
    return ((GameMode*)mGameMode)->GetGameType();
}

ServerPlayerGameMode* ServerPlayer::GetGameMode()
{
    return mGameMode;
}

void ServerPlayer::OnEquipArmor(not_null_ptr<ItemInstance> item)
{
    const SoundEvent* sound = SoundEvents::ARMOR_EQUIP_GENERIC;
    Item* heldItem = item->getItem();
    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(heldItem);
    if (armorItem != NULL)
    {
        sound = armorItem->getMaterial()->getEquipSound();
    }
    else if (heldItem == Items::elytra)
    {
        sound = SoundEvents::ARMOR_EQUIP_LEATHER;
    }

    mConnection->send(boost::shared_ptr<Packet>(new ClientboundSoundPacket(
        sound, SoundSource::eSoundSource_Neutral, mX, mY, mZ, 1.0f, 1.0f)));
}

int32_t ServerPlayer::checkTotemDeathProtection(DamageSource* parameterOne)
{
    int32_t result = LivingEntity::checkTotemDeathProtection(parameterOne);
    if (result == 0)
        return 0;

    boost::shared_ptr<Player> self = boost::static_pointer_cast<Player>(shared_from_this());
    not_null_ptr<ItemInstance> totemInstance(new ItemInstance(Items::totemOfUndying));
    awardStat(gStats->get_itemsUsed(Item::VALIDgetId(Items::totemOfUndying)),
              GenericStats::param_itemsUsed(self, totemInstance));
    return result;
}

uint64_t ServerPlayer::GetType()
{
    return 0x000000000C800001ULL;
}
