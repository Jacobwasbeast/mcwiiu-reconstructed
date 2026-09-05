#include "game/core/DamageSource.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/util/Vec3.hpp"

DamageSource::DamageSource(ClientboundChatPacket::EChatPacketMessage messageValue, ClientboundChatPacket::EChatPacketMessage deathMessageValue)
    : message(messageValue), deathMessage(deathMessageValue), bypassArmorFlag(false), bypassInvulFlag(false), bypassGraceInvulFlag(false), bypassMagicFlag(false), foodExhaustion(FoodConstants::ExhaustionAttack()), fire(false), projectile(false), scalesDifficulty(false), magic(false), explosion(false), fireProjectile(false)
{
}

DamageSource::DamageSource(const DamageSource& source)
    : message(source.message), deathMessage(source.deathMessage), bypassArmorFlag(source.bypassArmorFlag), bypassInvulFlag(source.bypassInvulFlag), bypassGraceInvulFlag(source.bypassGraceInvulFlag), bypassMagicFlag(source.bypassMagicFlag), foodExhaustion(source.foodExhaustion), fire(source.fire), projectile(source.projectile), scalesDifficulty(source.scalesDifficulty), magic(source.magic), explosion(source.explosion), fireProjectile(source.fireProjectile)
{
}

DamageSource::~DamageSource() { }

bool DamageSource::equals(const DamageSource* source) const { return message == source->message && deathMessage == source->deathMessage; }

boost::shared_ptr<EntityDamageSource> DamageSource::CreateMobAttack(boost::shared_ptr<LivingEntity> entity)
{
    return boost::shared_ptr<EntityDamageSource>(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1d), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1d), entity));
}

void DamageSource::setProjectile() { projectile = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source, const boost::shared_ptr<LivingEntity>& target, bool projectileValue)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1d), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1d), source, target, 0));
    if (projectileValue) result->setProjectile();
    return result;
}

boost::shared_ptr<EntityDamageSource> DamageSource::CreatePlayerAttack(boost::shared_ptr<Player> player)
{
    return boost::shared_ptr<EntityDamageSource>(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1e), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3c), player));
}

void DamageSource::setIsFireProjectile() { fireProjectile = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fireValue)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1f), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3d), arrow, owner, 0));
    result->setProjectile();
    if (fireValue) result->setIsFireProjectile();
    return result;
}

boost::shared_ptr<DamageSource> DamageSource::CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x2a), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x42), trident, owner, 0));
    result->setProjectile();
    return result;
}

void DamageSource::setIsFire() { fire = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateFireball(boost::shared_ptr<Fireball> fireball, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<DamageSource> result;
    if (owner)
        result.reset(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x20), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3d), fireball, owner, 0));
    else
        result.reset(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0xf), static_cast<ClientboundChatPacket::EChatPacketMessage>(0xf), fireball, fireball, 0));
    result->setIsFire();
    result->setProjectile();
    return result;
}

boost::shared_ptr<DamageSource> DamageSource::CreateThrown(boost::shared_ptr<Entity> projectileEntity, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x21), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3f), projectileEntity, owner, 0));
    result->setProjectile();
    return result;
}

void DamageSource::setMagic() { magic = true; }
void DamageSource::bypassArmor() { bypassArmorFlag = true; foodExhaustion = 0.0f; }

boost::shared_ptr<DamageSource> DamageSource::CreateIndirectMagic(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> target, const DamageSource* baseSource)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x22), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x43), source, target, baseSource));
    result->bypassArmor();
    result->setMagic();
    return result;
}

boost::shared_ptr<EntityDamageSource> DamageSource::CreateThorns(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x27), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x27), entity));
    result->setThorns();
    result->setMagic();
    return result;
}

void DamageSource::setExplosion() { explosion = true; }
void DamageSource::setScalesWithDifficulty() { scalesDifficulty = true; }

boost::shared_ptr<EntityDamageSource> DamageSource::CreateExplosion(Explosion* explosionValue)
{
    boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1b), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1b), boost::shared_ptr<Entity>()));
    result->setExplosion();
    result->setScalesWithDifficulty();
    return result;
}

boost::shared_ptr<EntityDamageSource> DamageSource::CreateExplosion(const boost::shared_ptr<LivingEntity>& entity)
{
    boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), entity));
    result->setExplosion();
    result->setScalesWithDifficulty();
    return result;
}

boost::shared_ptr<Entity> DamageSource::getDirectEntity() const { return getEntity(); }
boost::shared_ptr<Entity> DamageSource::getEntity() const { return boost::shared_ptr<Entity>(); }
bool DamageSource::isProjectile() const { return projectile; }
bool DamageSource::isExplosion() const { return explosion; }
bool DamageSource::isFireProjectile() const { return fireProjectile; }
bool DamageSource::isBypassArmor() const { return bypassArmorFlag; }
float DamageSource::getFoodExhaustion() const { return foodExhaustion; }
bool DamageSource::isBypassInvul() const { return bypassInvulFlag; }
bool DamageSource::isBypassGraceInvul() const { return bypassGraceInvulFlag; }
bool DamageSource::isBypassMagic() const { return bypassMagicFlag; }
bool DamageSource::isMagic() const { return magic; }
bool DamageSource::scalesWithDifficulty() const { return scalesDifficulty; }

void DamageSource::sendDamageIndicatorUpdate(boost::shared_ptr<ServerPlayer> player) const
{
    boost::shared_ptr<ClientboundDamageIndicatorPacket> packet(new ClientboundDamageIndicatorPacket());
    player->connection->send(packet);
}

boost::shared_ptr<ClientboundChatPacket> DamageSource::getDeathMessagePacket(boost::shared_ptr<LivingEntity> entity) const
{
    return boost::shared_ptr<ClientboundChatPacket>();
}

bool DamageSource::isFire() const { return fire; }
ClientboundChatPacket::EChatPacketMessage DamageSource::getMsgId() const { return message; }
bool DamageSource::isCreativePlayer() const { return false; }
boost::shared_ptr<Vec3> DamageSource::getSourcePosition() const { return boost::shared_ptr<Vec3>(); }
const DamageSource* DamageSource::GetBaseDamageSource() const { return 0; }
boost::shared_ptr<DamageSource> DamageSource::copy() const { return boost::shared_ptr<DamageSource>(new DamageSource(*this)); }
void DamageSource::bypassInvul() { bypassInvulFlag = true; }
void DamageSource::bypassGraceInvul() { bypassGraceInvulFlag = true; }
void DamageSource::bypassMagic() { bypassMagicFlag = true; foodExhaustion = 0.0f; }
