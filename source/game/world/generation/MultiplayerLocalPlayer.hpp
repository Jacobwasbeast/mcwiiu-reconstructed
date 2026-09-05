#ifndef GAME_WORLD_GENERATION_MULTIPLAYERLOCALPLAYER_HPP
#define GAME_WORLD_GENERATION_MULTIPLAYERLOCALPLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ClientPacketListener;
class DamageSource;
class ItemEntity;
class MobEffectInstance;
class Stat;
class Entity;

class MultiplayerLocalPlayer : public LocalPlayer
{
public:
    MultiplayerLocalPlayer(Minecraft* minecraft, Level* level, User* user, ClientPacketListener* packetListener);
    virtual ~MultiplayerLocalPlayer();

    virtual Level* GetLevel();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void heal(float amount);
    virtual void tick();
    virtual boost::shared_ptr<ItemEntity> drop();
    virtual void reallyDrop(boost::shared_ptr<ItemEntity> item);
    virtual void respawn();
    virtual void actuallyHurt(const DamageSource* source, float damage);
    virtual void completeUsingItem();
    virtual void onEffectAdded(MobEffectInstance* effect);
    virtual void onEffectUpdated(MobEffectInstance* effect, bool attributesChanged);
    virtual void onEffectRemoved(MobEffectInstance* effect);
    virtual void closeContainer();
    virtual void hurtTo(float amount, TelemetryDamageSource source);
    virtual void awardStat(Stat* stat, arrayWithLength<unsigned char> params);
    virtual bool isLocalPlayer();
    virtual bool startRiding(boost::shared_ptr<Entity> vehicle, bool force);
    virtual unsigned long long sendRidingJump();
    virtual unsigned long long sendOpenInventory();
    virtual void StopSleeping();
    virtual void setAndBroadcastCustomSkin(unsigned int skin);
    virtual void setAndBroadcastCustomCape(unsigned int cape);

    void fjResetBounceHeight();
    void clientSideCloseContainer();
    void awardStatFromServer(Stat* stat, arrayWithLength<unsigned char> params);
    void onSlimeBlockBounce();

private:
    bool m_hurt;
    int m_scoreboardWait;
    bool m_hasSentOpenInventory;
    bool m_hasSentRidingJump;
    bool m_receivedRespawn;
    int m_lastSentStat;
    double* m_bounceHeight;
    int m_bounceTicks;
};

#endif
