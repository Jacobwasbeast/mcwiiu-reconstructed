#include "game/core/DamageSource.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/Explosion.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

static const DamageSource* CreateOutOfWorldDamageSource()
{
    DamageSource* source = new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x18), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));
    source->bypassArmor();
    source->bypassInvul();
    return source;
}

static const DamageSource* CreateMagicDamageSource()
{
    DamageSource* source = new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1c), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));
    source->bypassArmor();
    source->setMagic();
    return source;
}

static const DamageSource* CreateLightningBoltDamageSource()
{
    DamageSource* source = new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(14), static_cast<ClientboundChatPacket::EChatPacketMessage>(53));
    source->bypassGraceInvul();
    return source;
}

static const DamageSource* CreateStarveDamageSource()
{
    DamageSource* source = new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x14), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));
    source->bypassArmor();
    source->bypassMagic();
    return source;
}

static const DamageSource* CreateCreativePlayerDamageSource()
{
    DamageSource* source = new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(25), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));
    source->bypassArmor();
    source->bypassInvul();
    return source;
}

const DamageSource* DamageSource::IN_FIRE =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x0d), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x34)))->setIsFire();
const DamageSource* DamageSource::LIGHTNING_BOLT = CreateLightningBoltDamageSource();
const DamageSource* DamageSource::ON_FIRE =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x0f), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x36)))->bypassArmor()->setIsFire();
const DamageSource* DamageSource::LAVA =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x10), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x37)))->setIsFire();
const DamageSource* DamageSource::HOT_FLOOR =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x40), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x41)))->setIsFire();
const DamageSource* DamageSource::IN_WALL =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x11), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::CRAMMING =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x12), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::DROWN =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x13), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x38)))->bypassArmor();
const DamageSource* DamageSource::STARVE = CreateStarveDamageSource();
const DamageSource* DamageSource::CACTUS =
    new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x15), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x39));
const DamageSource* DamageSource::FALL =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x16), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::OUT_OF_WORLD = CreateOutOfWorldDamageSource();
const DamageSource* DamageSource::CREATIVE_PLAYER = CreateCreativePlayerDamageSource();
const DamageSource* DamageSource::GENERIC =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1a), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::MAGIC = CreateMagicDamageSource();
const DamageSource* DamageSource::ANVIL =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x23), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::KILL =
    (new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3b), static_cast<ClientboundChatPacket::EChatPacketMessage>(0)))->bypassArmor();
const DamageSource* DamageSource::FALLING_BLOCK =
    new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x24), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));
const DamageSource* DamageSource::DRY_OUT =
    new DamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x29), static_cast<ClientboundChatPacket::EChatPacketMessage>(0));

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

EntityDamageSource* DamageSource::CreateMobAttack(boost::shared_ptr<LivingEntity> entity)
{
    return new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_MobAttack), static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_MobAttack), entity);
}

DamageSource* DamageSource::setProjectile() { projectile = true; return this; }

boost::shared_ptr<DamageSource> DamageSource::CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source, const boost::shared_ptr<LivingEntity>& target, bool projectileValue)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_MobAttack), static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_MobAttack), source, target, 0));
    if (projectileValue) result->setProjectile();
    return result;
}

boost::shared_ptr<EntityDamageSource> DamageSource::CreatePlayerAttack(boost::shared_ptr<Player> player)
{
    return boost::shared_ptr<EntityDamageSource>(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_PlayerAttack), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3c), player));
}

void DamageSource::setIsFireProjectile() { fireProjectile = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fireValue)
{
    boost::shared_ptr<Entity> arrowEntity(arrow, (Entity*)arrow.get());
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_Arrow), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3d), arrowEntity, owner, 0));
    result->setProjectile();
    if (fireValue) result->setIsFireProjectile();
    return result;
}

boost::shared_ptr<DamageSource> DamageSource::CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<Entity> tridentEntity(trident, (Entity*)trident.get());
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(eDamageMsgId_Trident), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x42), tridentEntity, owner, 0));
    result->setProjectile();
    return result;
}

DamageSource* DamageSource::setIsFire() { fire = true; return this; }

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

DamageSource* DamageSource::CreateThrown(boost::shared_ptr<Entity> projectileEntity, boost::shared_ptr<Entity> owner)
{
    DamageSource* result = new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x21), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3f), projectileEntity, owner, 0);
    result->setProjectile();
    return result;
}

void DamageSource::setMagic() { magic = true; }
DamageSource* DamageSource::bypassArmor() { bypassArmorFlag = true; foodExhaustion = 0.0f; return this; }

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
    if (explosionValue == 0 || !explosionValue->getSourceMob())
    {
        boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1b), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x1b), boost::shared_ptr<Entity>()));
        result->setExplosion();
        result->setScalesWithDifficulty();
        return result;
    }

    boost::shared_ptr<LivingEntity> sourceMob = explosionValue->getSourceMob();
    boost::shared_ptr<Entity> directSource = explosionValue->getDirectSource();
    if (directSource && directSource != sourceMob)
    {
        boost::shared_ptr<EntityDamageSource> result(new IndirectEntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), directSource, sourceMob, 0));
        result->setExplosion();
        result->setScalesWithDifficulty();
        return result;
    }

    boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), static_cast<ClientboundChatPacket::EChatPacketMessage>(0x3a), sourceMob));
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
    boost::shared_ptr<Entity> source = getEntity();
    boost::shared_ptr<ClientboundDamageIndicatorPacket> packet;
    if (source)
    {
        Vec3* direction = Vec3::newTemp(player->mX - source->mX, 0.0, player->mZ - source->mZ);
        direction->normalize();
        packet.reset(new ClientboundDamageIndicatorPacket((float)direction->x, (float)direction->z));
    }
    else
    {
        packet.reset(new ClientboundDamageIndicatorPacket());
    }
    player->mConnection->send(packet);
}

boost::shared_ptr<ClientboundChatPacket> DamageSource::getDeathMessagePacket(boost::shared_ptr<LivingEntity> entity) const
{
    boost::shared_ptr<LivingEntity> killer = entity->getKillCredit();
    if (killer)
    {
        std::wstring entityName = entity->getDisplayName();
        std::wstring killerName = killer->getDisplayName();
        ClientboundChatPacket::EChatPacketMessage packetMessage = deathMessage;
        if (packetMessage == static_cast<ClientboundChatPacket::EChatPacketMessage>(0))
            packetMessage = message;
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(entityName, packetMessage, killer, killerName));
    }

    std::wstring entityName = entity->getDisplayName();
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(entityName, message, -1));
}

bool DamageSource::isFire() const { return fire; }
ClientboundChatPacket::EChatPacketMessage DamageSource::getMsgId() const { return message; }
bool DamageSource::isCreativePlayer() const
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(getEntity());
    return player && player->mAbilities.mInstabuild;
}
Vec3* DamageSource::getSourcePosition() const { return 0; }
const DamageSource* DamageSource::GetBaseDamageSource() const { return 0; }
DamageSource* DamageSource::copy() const { return new DamageSource(*this); }
void DamageSource::bypassInvul() { bypassInvulFlag = true; }
void DamageSource::bypassGraceInvul() { bypassGraceInvulFlag = true; }
void DamageSource::bypassMagic() { bypassMagicFlag = true; foodExhaustion = 0.0f; }
