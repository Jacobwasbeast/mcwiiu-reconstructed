#ifndef GAME_CORE_DAMAGESOURCE_HPP
#define GAME_CORE_DAMAGESOURCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/network/ClientboundChatPacket.hpp"

class Arrow;
class Entity;
class EntityDamageSource;
class Explosion;
class Fireball;
class LivingEntity;
class Player;
class ServerPlayer;
class ThrownTrident;
class Vec3;

class DamageSource
{
public:
    static const DamageSource* IN_FIRE;
    static const DamageSource* ON_FIRE;
    static const DamageSource* OUT_OF_WORLD;
    static const DamageSource* IN_WALL;
    static const DamageSource* LAVA;
    static const DamageSource* HOT_FLOOR;
    static const DamageSource* ANVIL;
    static const DamageSource* FALLING_BLOCK;
    static const DamageSource* KILL;
    static const DamageSource* GENERIC;
    static const DamageSource* MAGIC;
    static const DamageSource* CRAMMING;
    static const DamageSource* DROWN;
    static const DamageSource* FALL;

    DamageSource(ClientboundChatPacket::EChatPacketMessage message, ClientboundChatPacket::EChatPacketMessage deathMessage);
    DamageSource(const DamageSource& source);
    virtual ~DamageSource();
    bool equals(const DamageSource* source) const;
    static boost::shared_ptr<EntityDamageSource> CreateMobAttack(boost::shared_ptr<LivingEntity> entity);
    DamageSource* setProjectile();
    static boost::shared_ptr<DamageSource> CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source, const boost::shared_ptr<LivingEntity>& target, bool projectile);
    static boost::shared_ptr<EntityDamageSource> CreatePlayerAttack(boost::shared_ptr<Player> player);
    void setIsFireProjectile();
    static boost::shared_ptr<DamageSource> CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fire);
    static boost::shared_ptr<DamageSource> CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner);
    DamageSource* setIsFire();
    static boost::shared_ptr<DamageSource> CreateFireball(boost::shared_ptr<Fireball> fireball, boost::shared_ptr<Entity> owner);
    static DamageSource* CreateThrown(boost::shared_ptr<Entity> projectile, boost::shared_ptr<Entity> owner);
    void setMagic();
    DamageSource* bypassArmor();
    static boost::shared_ptr<DamageSource> CreateIndirectMagic(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> target, const DamageSource* baseSource);
    static boost::shared_ptr<EntityDamageSource> CreateThorns(boost::shared_ptr<Entity> entity);
    void setExplosion();
    void setScalesWithDifficulty();
    static boost::shared_ptr<EntityDamageSource> CreateExplosion(Explosion* explosion);
    static boost::shared_ptr<EntityDamageSource> CreateExplosion(const boost::shared_ptr<LivingEntity>& entity);
    virtual boost::shared_ptr<Entity> getDirectEntity() const;
    virtual boost::shared_ptr<Entity> getEntity() const;
    bool isProjectile() const;
    bool isExplosion() const;
    bool isFireProjectile() const;
    bool isBypassArmor() const;
    float getFoodExhaustion() const;
    bool isBypassInvul() const;
    bool isBypassGraceInvul() const;
    bool isBypassMagic() const;
    bool isMagic() const;
    virtual bool scalesWithDifficulty() const;
    void sendDamageIndicatorUpdate(boost::shared_ptr<ServerPlayer> player) const;
    virtual boost::shared_ptr<ClientboundChatPacket> getDeathMessagePacket(boost::shared_ptr<LivingEntity> entity) const;
    bool isFire() const;
    ClientboundChatPacket::EChatPacketMessage getMsgId() const;
    bool isCreativePlayer() const;
    virtual Vec3* getSourcePosition() const;
    virtual const DamageSource* GetBaseDamageSource() const;
    virtual boost::shared_ptr<DamageSource> copy() const;
    void bypassInvul();
    void bypassGraceInvul();
    void bypassMagic();

protected:
    ClientboundChatPacket::EChatPacketMessage message;
    ClientboundChatPacket::EChatPacketMessage deathMessage;
    bool bypassArmorFlag;
    bool bypassInvulFlag;
    bool bypassGraceInvulFlag;
    bool bypassMagicFlag;
    float foodExhaustion;
    bool fire;
    bool projectile;
    bool scalesDifficulty;
    bool magic;
    bool explosion;
    bool fireProjectile;
};

#endif
