#include "game/world/generation/MultiplayerLocalPlayer.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Stat.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"

MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft* minecraft, Level* level, User* user, ClientPacketListener* packetListener)
    : LocalPlayer(minecraft, level, user, 0),
      m_hurt(false), m_scoreboardWait(-1), m_hasSentOpenInventory(false),
      m_hasSentRidingJump(false), m_receivedRespawn(false), m_lastSentStat(0),
      m_bounceHeight(0), m_bounceTicks(0)
{
    (void)packetListener;
}

MultiplayerLocalPlayer::~MultiplayerLocalPlayer()
{
    delete m_bounceHeight;
    m_bounceHeight = 0;
}

MultiPlayerLevel* MultiplayerLocalPlayer::GetLevel()
{
    return dynamic_cast<MultiPlayerLevel*>(mLevel);
}

const GameType* MultiplayerLocalPlayer::GetGameType()
{
    MultiPlayerGameMode* gameMode = GetLocalGameMode();
    return gameMode == NULL ? NULL : gameMode->GetGameType();
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
    delete m_bounceHeight;
    m_bounceHeight = 0;
}

void MultiplayerLocalPlayer::tick()
{
    if (mLevel == 0)
        return;
    LocalPlayer::tick();
    if (m_scoreboardWait >= 0)
    {
        --m_scoreboardWait;
        if (m_scoreboardWait == 0)
            SetWaitingForScoreboard(true);
    }
    if (m_bounceHeight != 0 && m_bounceTicks == 0)
        fjResetBounceHeight();
}

boost::shared_ptr<ItemEntity> MultiplayerLocalPlayer::drop()
{
    return LocalPlayer::drop(false);
}

void MultiplayerLocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    if (item)
        LocalPlayer::reallyDrop(item);
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
    if (source == 0 || !isInvulnerableTo(source) || source->isBypassInvul())
        LivingEntity::actuallyHurt(source, damage);
}

void MultiplayerLocalPlayer::completeUsingItem()
{
    LivingEntity::completeUsingItem();
}

void MultiplayerLocalPlayer::onEffectAdded(MobEffectInstance* effect)
{
    LivingEntity::onEffectAdded(effect);
}

void MultiplayerLocalPlayer::onEffectUpdated(MobEffectInstance* effect, bool attributesChanged)
{
    LivingEntity::onEffectUpdated(effect, attributesChanged);
}

void MultiplayerLocalPlayer::onEffectRemoved(MobEffectInstance* effect)
{
    LivingEntity::onEffectRemoved(effect);
}

void MultiplayerLocalPlayer::clientSideCloseContainer()
{
    LocalPlayer::clientSideCloseContainer();
}

void MultiplayerLocalPlayer::closeContainer()
{
    LocalPlayer::closeContainer();
    clientSideCloseContainer();
}

void MultiplayerLocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    if (m_hurt)
        LocalPlayer::hurtTo(amount, source);
    else
        m_hurt = true;
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
    fjResetBounceHeight();
    m_bounceHeight = new double(0.0);
}

unsigned long long MultiplayerLocalPlayer::sendRidingJump()
{
    return LocalPlayer::sendRidingJump();
}

unsigned long long MultiplayerLocalPlayer::sendOpenInventory()
{
    return LocalPlayer::sendOpenInventory();
}

bool MultiplayerLocalPlayer::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    return LocalPlayer::startRiding(vehicle, force);
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
    LocalPlayer::setAndBroadcastCustomSkin(skin);
}

void MultiplayerLocalPlayer::setAndBroadcastCustomCape(unsigned int cape)
{
    LocalPlayer::setAndBroadcastCustomCape(cape);
}
