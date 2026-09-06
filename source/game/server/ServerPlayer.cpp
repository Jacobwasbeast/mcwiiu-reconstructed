#include "game/server/ServerPlayer.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/core/Stat.hpp"
#include "game/item/Inventory.hpp"
#include "game/network/ClientboundAwardStatPacket.hpp"
#include "game/network/ClientboundContainerSetContentPacket.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

ServerPlayer::ServerPlayer(MinecraftServer* server, ServerLevel* level, const std::wstring& name, ServerPlayerGameMode* gameMode)
    : Player((Level*)level, name), mConnection(), mServer(server), mGameMode(gameMode),
      mLoadedChunks(), mSentEntityIds(), mTrackedChunks(), mViewDistance(0.0f), mChunkX(0), mChunkZ(0), mChunkRadius(0),
      mChunkIndex(0), mChunkFlags(0), mLastActionTime(0.0f), mDimensionChangeDelay(0), mSpectatorState(0), mExperienceLevel(0),
      mExperienceTotal(0), mLastTick(0), mRemoveEntityCount(0), mRespawnDimension(0), mEntitiesToRemove(0), mChangingDimension(0),
      mSpawnInvulnerableTime(0), mPvpState(0), mViewDistanceModifier(0), mIsChangingQuantityOnly(false), latency(0), mFallFlying(0),
      mRespawnState(false), mChangedDimension(0), mTeleportPending(0), mCameraEntity(0), mType(0)
{
}

ServerPlayer::~ServerPlayer()
{
}

void ServerPlayer::sendMessage(uint32_t parameterOne, int32_t* parameterTwo, uint32_t parameterThree, std::wstring* parameterFour, uint32_t parameterFive)
{
}

uint32_t ServerPlayer::hasPermission(uint32_t parameterOne)
{
    return 0;
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
    return 0;
}

uint64_t ServerPlayer::setAllContainerData(AbstractContainerMenu* parameterOne, boost::shared_ptr<Container>* parameterTwo)
{
    return 0;
}

uint64_t ServerPlayer::flagEntitiesToBeRemoved(uint32_t* parameterOne, bool* parameterTwo)
{
    return 0;
}

int32_t ServerPlayer::getFlagIndexForChunk(ChunkPos* parameterOne, int32_t parameterTwo)
{
    return 0;
}

void ServerPlayer::disconnect()
{
}

ServerLevel* ServerPlayer::getLevel()
{
    return static_cast<ServerLevel*>(mLevel);
}

void ServerPlayer::doTickA()
{
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
    return 0;
}

void ServerPlayer::doTick(bool parameterOne, bool parameterTwo, bool parameterThree)
{
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
}

uint64_t ServerPlayer::notifyPlayerRespawn(boost::shared_ptr<ServerPlayer>* parameterOne, boost::shared_ptr<ServerPlayer>* parameterTwo)
{
    return 0;
}

void ServerPlayer::flushEntitiesToRemove()
{
}

uint64_t ServerPlayer::markAttributesDirty()
{
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
    return 0;
}

void ServerPlayer::resetChunkFlagsOutsideArea(int32_t parameterOne, int32_t parameterTwo, int32_t parameterThree, int32_t parameterFour, int32_t parameterFive)
{
}

void ServerPlayer::readAdditionalSaveData(CompoundTag* parameterOne)
{
}

void ServerPlayer::addWalkers(DataFixerUpper* parameterOne)
{
}

void ServerPlayer::addAdditonalSaveData(CompoundTag* parameterOne)
{
}

void ServerPlayer::giveExperienceLevels(int32_t parameterOne)
{
}

void ServerPlayer::onEnchantmentPerformed(int32_t parameterOne)
{
}

void ServerPlayer::onEnterCombat()
{
}

void ServerPlayer::onLeaveCombat()
{
}

void ServerPlayer::ResetSpawnInvulnerableTime()
{
}

void ServerPlayer::tick()
{
}

uint64_t ServerPlayer::die(DamageSource* parameterOne)
{
    return 0;
}

int32_t ServerPlayer::hurt(uint64_t parameterOne, float parameterTwo)
{
    return 0;
}

void ServerPlayer::isPvpAllowed()
{
}

uint32_t ServerPlayer::canHarmPlayer(boost::shared_ptr<Player>* parameterOne)
{
    return 0;
}

int32_t ServerPlayer::canHarmPlayer(std::wstring* parameterOne)
{
    return 0;
}

void ServerPlayer::addEffect(MobEffectInstance* parameterOne, const boost::shared_ptr<Entity>& parameterTwo)
{
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
    return 0;
}

int32_t ServerPlayer::startSleepInBed(BlockPos* parameterOne, bool parameterTwo)
{
    return 0;
}

uint64_t ServerPlayer::stopSleepInBed(bool parameterOne, bool parameterTwo, bool parameterThree)
{
    return 0;
}

uint32_t ServerPlayer::startRiding(boost::shared_ptr<Entity>* parameterOne)
{
    return 0;
}

void ServerPlayer::stopRiding()
{
}

void ServerPlayer::stopRidingBeforeTeleport()
{
}

uint8_t ServerPlayer::isChangingDimension()
{
    return 0;
}

uint32_t ServerPlayer::isInvulnerableTo(DamageSource* parameterOne)
{
    return 0;
}

void ServerPlayer::checkFallDamage(double parameterOne, bool parameterTwo, Block* parameterThree, BlockPos* parameterFour)
{
}

uint64_t ServerPlayer::doCheckFallDamage(double parameterOne, bool parameterTwo)
{
    return 0;
}

void ServerPlayer::nextContainerCounter()
{
}

void ServerPlayer::openTextEdit(boost::shared_ptr<SignBlockEntity> parameterOne)
{
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
}

void ServerPlayer::closeContainer()
{
}

void ServerPlayer::broadcastCarriedItem()
{
}

void ServerPlayer::setPlayerInput(float parameterOne, float parameterTwo, bool parameterThree, bool parameterFour)
{
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
    return 0;
}

uint64_t ServerPlayer::displayClientMessage(int32_t parameterOne, bool parameterTwo)
{
    return 0;
}

void ServerPlayer::completeUsingItem()
{
}

uint64_t ServerPlayer::restoreFrom(boost::shared_ptr<Player>* parameterOne, bool parameterTwo)
{
    return 0;
}

void ServerPlayer::onEffectAdded(MobEffectInstance* parameterOne)
{
}

void ServerPlayer::onEffectUpdated(MobEffectInstance* parameterOne, bool parameterTwo)
{
}

void ServerPlayer::onEffectRemoved(MobEffectInstance* parameterOne)
{
}

void ServerPlayer::teleportTo(double parameterOne, double parameterTwo, double parameterThree)
{
}

void ServerPlayer::teleportTo(double parameterOne, double parameterTwo, double parameterThree, float parameterFour, float parameterFive)
{
}

uint64_t ServerPlayer::crit(boost::shared_ptr<Entity>* parameterOne)
{
    return 0;
}

uint64_t ServerPlayer::magicCrit(boost::shared_ptr<Entity>* parameterOne)
{
    return 0;
}

void ServerPlayer::onUpdateAbilities()
{
}

void ServerPlayer::setGameMode(GameType* parameterOne)
{
}

bool ServerPlayer::isSpectator()
{
    return mSpectatorState != 0;
}

bool ServerPlayer::isCreative()
{
    return mPvpState != 0;
}

void ServerPlayer::resetLastActionTime()
{
}

uint64_t ServerPlayer::sendRemoveEntity(boost::shared_ptr<Entity>* parameterOne)
{
    return 0;
}

uint64_t ServerPlayer::cancelRemoveEntity(boost::shared_ptr<Entity>* parameterOne)
{
    return 0;
}

void ServerPlayer::updateInvisibilityStatus()
{
}

void ServerPlayer::processDimensionDelay()
{
}

uint64_t ServerPlayer::attack(boost::shared_ptr<Entity> parameterOne)
{
    return 0;
}

bool ServerPlayer::swing(InteractionHand::EInteractionHand parameterOne)
{
    return 0;
}

void ServerPlayer::hasChangedDimension()
{
}

void ServerPlayer::startFallFlying()
{
}

void ServerPlayer::stopFallFlying()
{
}

uint32_t ServerPlayer::getPlayerViewDistanceModifier()
{
    return 0;
}

void ServerPlayer::handleCollectItem(not_null_ptr_ItemInstance* parameterOne)
{
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
    return 0;
}
