#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/EntityType.hpp"

#include "game/item/ItemInstance.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Stat.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Input.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/core/GameSettings.hpp"
#include "game/network/TextureChangePacket.hpp"
#include "game/network/TextureAndGeometryChangePacket.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft* minecraft, Level* level, User* user, ClientPacketListener* packetListener)
    : LocalPlayer(minecraft, level, user, level->mDimension->getType()->getId()),
      mHurt(false), mScoreboardWait(-1), mHasSentOpenInventory(false),
      mHasSentRidingJump(false), mReceivedRespawn(false), mLastSentStat(0),
      mBounceHeight(0), mFreeDiverTicks(0)
{
    LivingEntity::fjDerivedCtorCalls();
    mPacketListener = packetListener;
}

MultiplayerLocalPlayer::~MultiplayerLocalPlayer()
{
    delete mBounceHeight;
    mBounceHeight = 0;
}

MultiPlayerLevel* MultiplayerLocalPlayer::GetLevel()
{
    return dynamic_cast<MultiPlayerLevel*>(mLevel);
}

bool MultiplayerLocalPlayer::hurt(const DamageSource* source, float damage)
{
    return false;
}

void MultiplayerLocalPlayer::heal(float amount)
{
}

void MultiplayerLocalPlayer::fjResetBounceHeight()
{
    delete mBounceHeight;
    mBounceHeight = 0;
}

void MultiplayerLocalPlayer::tick()
{
    if (!mLevel->hasChunkAt(BlockPos(Mth::floor(mX), 0, Mth::floor(mZ))))
        return;
    LocalPlayer::tick();

    if (isAlive() && (gMinecraftServer == NULL || !gMinecraftServer->IsServerPaused()))
    {
        if (isUnderLiquid(Material::WATER))
        {
            if (mFreeDiverTicks == 0)
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::AMBIENT_UNDERWATER_ENTER,
                                       SoundSource::eSoundSource_Ambient, 1.0f, 1.0f, false, 16.0f);
            }
            ++mFreeDiverTicks;
            if (mFreeDiverTicks / 20 > 120)
                awardStat(GenericStats::freeDiver(), GenericStats::param_freeDiver(mFreeDiverTicks / 20));
        }
        else
        {
            if (mFreeDiverTicks != 0)
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::AMBIENT_UNDERWATER_EXIT,
                                       SoundSource::eSoundSource_Ambient, 1.0f, 1.0f, false, 16.0f);
            }
            mFreeDiverTicks = 0;
        }
    }

    if (mBounceHeight != 0)
    {
        int heightGained = (int)(mY - *mBounceHeight);
        if (heightGained >= 30)
            awardStat(GenericStats::trampoline(), GenericStats::param_trampoline(heightGained));
        if (mYd <= 0.0)
            fjResetBounceHeight();
    }

    if (mScoreboardWait < 0)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
            if (gameMode != NULL && gameMode->IsRoundPlaying())
            {
                bool endZoneEntered = GetLevel()->GetClientScoreboard()->GetEndZoneEntered(getSUID(), -1);
                bool gliding = isFallFlying() && !isInWater() && !isInLava();
                if (endZoneEntered && !isSpectator() && !gliding)
                    mScoreboardWait = 40;
            }
        }
    }
    else if (--mScoreboardWait == 0)
    {
        SetWaitingForScoreboard(true);
        StartSpectating();
    }
}

boost::shared_ptr<ItemEntity> MultiplayerLocalPlayer::drop()
{
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(
        ServerboundPlayerActionPacket::DROP_ITEM, BlockPos(0, 0, 0), Direction::DOWN, 0)));
    return boost::shared_ptr<ItemEntity>();
}

not_null_ptr<ItemInstance> MultiplayerLocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    return ItemInstance::EMPTY;
}

void MultiplayerLocalPlayer::respawn()
{
    if (mPacketListener == NULL)
        return;
    boost::shared_ptr<Packet> packet(new ServerboundClientCommandPacket(
        ServerboundClientCommandPacket::PERFORM_RESPAWN));
    mPacketListener->send(packet);
}

void MultiplayerLocalPlayer::actuallyHurt(const DamageSource* source, float damage)
{
    if (!isInvulnerableTo(source) || source->isBypassInvul())
        setHealth(getHealth() - damage);
}

void MultiplayerLocalPlayer::completeUsingItem()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!mUseItem->isEmpty() && minecraft->mGameModes[mPlayerIndex] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[mPlayerIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->completeUsingItem(mUseItem);
    }
    if (mUseItem->isEmpty())
        mUseItem = getCarriedItem();
    LivingEntity::completeUsingItem();
}

void MultiplayerLocalPlayer::onEffectAdded(MobEffectInstance* effect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mPlayerIndex] != NULL && minecraft->mGameModes[mPlayerIndex]->getTutorial() != NULL)
        minecraft->mGameModes[mPlayerIndex]->getTutorial()->onEffectChanged(effect->getEffect(), false);
    LivingEntity::onEffectAdded(effect);
}

void MultiplayerLocalPlayer::onEffectUpdated(MobEffectInstance* effect, bool attributesChanged)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mPlayerIndex] != NULL && minecraft->mGameModes[mPlayerIndex]->getTutorial() != NULL)
        minecraft->mGameModes[mPlayerIndex]->getTutorial()->onEffectChanged(effect->getEffect(), false);
    LivingEntity::onEffectUpdated(effect, attributesChanged);
}

void MultiplayerLocalPlayer::onEffectRemoved(MobEffectInstance* effect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mPlayerIndex] != NULL && minecraft->mGameModes[mPlayerIndex]->getTutorial() != NULL)
        minecraft->mGameModes[mPlayerIndex]->getTutorial()->onEffectChanged(effect->getEffect(), true);
    LivingEntity::onEffectRemoved(effect);
}

void MultiplayerLocalPlayer::clientSideCloseContainer()
{
    mInventory->setCarried(ItemInstance::EMPTY);
    LocalPlayer::clientSideCloseContainer();
}

void MultiplayerLocalPlayer::closeContainer()
{
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundContainerClosePacket(mContainerMenu->m_containerId)));
    MultiplayerLocalPlayer::clientSideCloseContainer();
}

void MultiplayerLocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    if (mHurt)
        LocalPlayer::hurtTo(amount, source);
    else
    {
        setHealth(amount);
        mHurt = true;
    }
}

void MultiplayerLocalPlayer::awardStat(Stat* stat, arrayWithLength<unsigned char> params)
{
    if (stat != 0 && !stat->isAwardedFromServer())
        LocalPlayer::awardStat(stat, params);
}

void MultiplayerLocalPlayer::awardStatFromServer(Stat* stat, arrayWithLength<unsigned char> params)
{
    if (stat != 0 && stat->isAwardedFromServer())
        LocalPlayer::awardStat(stat, params);
}

bool MultiplayerLocalPlayer::isLocalPlayer()
{
    return true;
}

void MultiplayerLocalPlayer::onSlimeBlockBounce()
{
    delete mBounceHeight;
    mBounceHeight = new double(mY);
}

void MultiplayerLocalPlayer::sendRidingJump()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
        entity, ServerboundPlayerCommandPacket::RIDING_JUMP, static_cast<int>(getJumpRidingScale() * 100.0f)));
    mPacketListener->send(packet);
}

void MultiplayerLocalPlayer::sendOpenInventory()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
        entity, ServerboundPlayerCommandPacket::OPEN_INVENTORY));
    mPacketListener->send(packet);
}

bool MultiplayerLocalPlayer::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    bool wasRiding = isPassenger();
    bool result = LocalPlayer::startRiding(vehicle, force);
    bool riding = isPassenger();
    if (riding)
    {
        int challenge = 0;
        if (getVehicle() != NULL)
        {
            unsigned long long type = getVehicle()->GetType();
            if (type == EntityType::PIG)
                challenge = 0x12;
            else if (type == EntityType::MINECART_RIDEABLE || type == EntityType::MINECART)
                challenge = 0x10;
            else if (type == EntityType::BOAT)
                challenge = 0x11;
        }
        telemetryManagerInstance->RecordEnemyKilledOrOvercome(mPlayerIndex, (int)mX, (int)mY, (int)mZ, 0, 0, 0,
                                                              static_cast<ETelemetryChallenges>(challenge));
    }
    else if (wasRiding)
    {
        setSneaking(false);
        mInput->mSneaking = false;
    }
    updateRichPresence();

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[mPlayerIndex] != NULL)
    {
        if (!wasRiding)
        {
            Tutorial* tutorial = riding ? minecraft->mGameModes[mPlayerIndex]->getTutorial() : NULL;
            if (tutorial != NULL)
                tutorial->onRideEntity(vehicle);
        }
        else if (!riding)
        {
            Tutorial* tutorial = minecraft->mGameModes[mPlayerIndex]->getTutorial();
            if (tutorial != NULL)
                tutorial->changeTutorialState(eTutorial_State_Gameplay, NULL);
        }
    }
    return result;
}

void MultiplayerLocalPlayer::StopSleeping()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(new ServerboundPlayerCommandPacket(
        entity, ServerboundPlayerCommandPacket::STOP_SLEEPING));
    mPacketListener->send(packet);
}

void MultiplayerLocalPlayer::setAndBroadcastCustomSkin(unsigned int skin)
{
    int oldSkin = mCustomSkin;
    Player::setCustomSkin(skin);
    if (mCustomSkin == oldSkin)
        return;

    boost::shared_ptr<Entity> entity = shared_from_this();
    mPacketListener->send(boost::shared_ptr<Packet>(
        new TextureAndGeometryChangePacket(entity, GameSettings::GetPlayerSkinName(mPlayerIndex))));
}

void MultiplayerLocalPlayer::setAndBroadcastCustomCape(unsigned int cape)
{
    int oldCape = mCustomCape;
    Player::setCustomCape(cape);
    if (mCustomCape == oldCape)
        return;

    boost::shared_ptr<Entity> entity = shared_from_this();
    mPacketListener->send(boost::shared_ptr<Packet>(new TextureChangePacket(
        entity, TextureChangePacket::eTextureChangeType_Cape, GameSettings::GetPlayerCapeName(mPlayerIndex))));
}
