#include "game/server/ServerPlayer.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/core/Stat.hpp"
#include "game/item/Inventory.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ClientboundContainerSetDataPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundSetCameraPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundRemoveEntitiesPacket.hpp"
#include "game/network/ClientboundContainerClosePacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/System.hpp"
#include "game/core/GameType.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/core/GameRulesInstance.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/ModifiableAttributeInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/DataWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include <cstring>

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
}

ServerPlayer::ServerPlayer(MinecraftServer* server, ServerLevel* level, const std::wstring& name, ServerPlayerGameMode* gameMode)
    : Player((Level*)level, name), mConnection(), mServer(server), mGameMode(gameMode),
      mLoadedChunks(), mSentEntityIds(), mTrackedChunks(), mViewDistance(0.0f), mChunkX(0), mChunkZ(0), mChunkRadius(0),
      mChunkIndex(0), mChunkFlags(0), mLastActionTime(0.0f), mDimensionChangeDelay(0), mSpectatorState(0), mExperienceLevel(0),
      mExperienceTotal(0), mLastTick(0), mRemoveEntityCount(0), mRespawnDimension(0), mEntitiesToRemove(0), mLevitationSourceY(NULL), mChangingDimension(0),
      mSpawnInvulnerableTime(0), mPvpState(0), mViewDistanceModifier(0), mIsChangingQuantityOnly(false), latency(0), mFallFlying(0),
      mRespawnState(false), mChangedDimension(0), mTeleportPending(0), mCameraEntity(0), mType(0)
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
    return BlockPos();
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

uint64_t ServerPlayer::setContainerData(AbstractContainerMenu* parameterOne, int32_t parameterTwo, int32_t parameterThree)
{
    if (mConnection != NULL && parameterOne != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetDataPacket(parameterOne->m_containerId, parameterTwo, parameterThree)));
    return 0;
}

uint64_t ServerPlayer::setAllContainerData(AbstractContainerMenu* parameterOne, boost::shared_ptr<Container>* parameterTwo)
{
    Container& container = **parameterTwo;
    for (int index = 0; index < container.getContainerDataCount(); ++index)
    {
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerSetDataPacket(
            parameterOne->m_containerId, index, container.getContainerData(index))));
    }
    return 0;
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
    mDisconnected = true;
}

ServerLevel* ServerPlayer::getLevel()
{
    return static_cast<ServerLevel*>(mLevel);
}

void ServerPlayer::doTickA()
{
    Player::tick();
}

uint64_t ServerPlayer::broadcast(boost::shared_ptr<BlockEntity>* parameterOne, bool parameterTwo)
{
    return 0;
}

void ServerPlayer::doChunkSendingTick(bool parameterOne, bool parameterTwo)
{
}

uint64_t ServerPlayer::doTickB()
{
    processDimensionDelay();
    return 0;
}

void ServerPlayer::doTick(bool parameterOne, bool parameterTwo, bool parameterThree)
{
    if (parameterOne)
        updateFrameTick();
    doTickA();
    if (parameterOne)
        doChunkSendingTick(parameterTwo, false);
    doTickB();
}

void ServerPlayer::initMenu()
{
}

uint64_t ServerPlayer::SpectatePlayerChange(uint32_t parameterOne, int32_t parameterTwo)
{
    return 0;
}

void ServerPlayer::updateClientsCameras()
{
    if (mConnection != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundSetCameraPacket(mCamera)));
}

uint64_t ServerPlayer::notifyPlayerRespawn(boost::shared_ptr<ServerPlayer>* parameterOne, boost::shared_ptr<ServerPlayer>* parameterTwo)
{
    return 0;
}

void ServerPlayer::flushEntitiesToRemove()
{
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
}

void ServerPlayer::readAdditionalSaveData(CompoundTag* parameterOne)
{
    Player::readAdditionalSaveData(parameterOne);
}

void ServerPlayer::addWalkers(DataFixerUpper* parameterOne)
{
    parameterOne->addWalker(DataFixTypes::eDataFixTypes_Player, new ServerPlayerDataWalker());
}

void ServerPlayer::addAdditonalSaveData(CompoundTag* parameterOne)
{
    Player::addAdditonalSaveData(parameterOne);
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
}

void ServerPlayer::onLeaveCombat()
{
    LivingEntity::onLeaveCombat();
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
    if (mChangingDimension)
        return 0;
    return 0;
}

void ServerPlayer::isPvpAllowed()
{
    if (mServer != NULL)
        mServer->isPvpAllowed();
}

uint32_t ServerPlayer::canHarmPlayer(boost::shared_ptr<Player>* parameterOne)
{
    if (parameterOne == NULL || mServer == NULL || !mServer->isPvpAllowed() ||
        !isAllowedToAttackPlayers())
        return 0;
    return Player::canHarmPlayer(*parameterOne);
}

int32_t ServerPlayer::canHarmPlayer(std::wstring* parameterOne)
{
    if (parameterOne == NULL || mServer == NULL || !mServer->isPvpAllowed())
        return 0;
    return Player::canHarmPlayer(*parameterOne);
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

uint64_t ServerPlayer::changeDimension(uint32_t parameterOne, int32_t parameterTwo)
{
    return 0;
}

uint32_t ServerPlayer::broadcastToPlayer(boost::shared_ptr<ServerPlayer>* parameterOne)
{
    return 0;
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
    Player::stopSleepInBed(parameterOne, parameterTwo, parameterThree);
    return 0;
}

uint32_t ServerPlayer::startRiding(boost::shared_ptr<Entity>* parameterOne)
{
    return parameterOne != NULL && Player::startRiding(*parameterOne);
}

void ServerPlayer::stopRiding()
{
    Player::stopRiding();
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

uint32_t ServerPlayer::isInvulnerableTo(DamageSource* parameterOne)
{
    return Entity::isInvulnerableTo(parameterOne) || mChangingDimension;
}

void ServerPlayer::checkFallDamage(double parameterOne, bool parameterTwo, Block* parameterThree, BlockPos* parameterFour)
{
}

uint64_t ServerPlayer::doCheckFallDamage(double parameterOne, bool parameterTwo)
{
    checkFallDamage(parameterOne, parameterTwo, NULL, NULL);
    return 0;
}

void ServerPlayer::nextContainerCounter()
{
}

void ServerPlayer::openTextEdit(boost::shared_ptr<SignBlockEntity> parameterOne)
{
    Player::openTextEdit(parameterOne);
}

uint32_t ServerPlayer::startBlockInteraction(InteractionObject* parameterOne)
{
    return 0;
}

uint32_t ServerPlayer::openFireworks(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree)
{
    return 0;
}

uint32_t ServerPlayer::openContainer(boost::shared_ptr<Container>* parameterOne)
{
    return 0;
}

uint32_t ServerPlayer::openTrading(uint32_t parameterOne, std::wstring* parameterTwo, int32_t parameterThree)
{
    if (mContainerMenu != mInventoryMenu)
        return 0;
    return 0;
}

uint32_t ServerPlayer::openHorseInventory(uint32_t parameterOne, boost::shared_ptr<Container>* parameterTwo)
{
    return 0;
}

void ServerPlayer::openItemInstanceGui(not_null_ptr_ItemInstance* parameterOne, uint32_t parameterTwo)
{
}

void ServerPlayer::doCloseContainer()
{
    if (mConnection != NULL && mContainerMenu != NULL)
        mConnection->send(boost::shared_ptr<Packet>(new ClientboundContainerClosePacket(mContainerMenu->m_containerId)));
}

void ServerPlayer::closeContainer()
{
    Player::closeContainer();
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

uint64_t ServerPlayer::displayClientMessage(int32_t parameterOne, bool parameterTwo)
{
    Player::displayClientMessage(parameterOne, parameterTwo);
    return 0;
}

void ServerPlayer::completeUsingItem()
{
    LivingEntity::completeUsingItem();
}

uint64_t ServerPlayer::restoreFrom(boost::shared_ptr<Player>* parameterOne, bool parameterTwo)
{
    return 0;
}

void ServerPlayer::onEffectAdded(MobEffectInstance* parameterOne)
{
    LivingEntity::onEffectAdded(parameterOne);
}

void ServerPlayer::onEffectUpdated(MobEffectInstance* parameterOne, bool parameterTwo)
{
    LivingEntity::onEffectUpdated(parameterOne, parameterTwo);
}

void ServerPlayer::onEffectRemoved(MobEffectInstance* parameterOne)
{
    LivingEntity::onEffectRemoved(parameterOne);
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
    if (parameterOne != NULL)
        Player::crit(*parameterOne);
    return 0;
}

uint64_t ServerPlayer::magicCrit(boost::shared_ptr<Entity>* parameterOne)
{
    if (parameterOne != NULL)
        Player::magicCrit(*parameterOne);
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
    if (mConnection != NULL && parameterOne != NULL && *parameterOne)
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

uint64_t ServerPlayer::OnEquipArmor(not_null_ptr_ItemInstance* parameterOne)
{
    return 0;
}

int32_t ServerPlayer::checkTotemDeathProtection(DamageSource* parameterOne)
{
    return 0;
}

uint64_t ServerPlayer::GetType()
{
    return 0x000000000C800001ULL;
}
