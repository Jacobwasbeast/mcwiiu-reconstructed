#include "game/core/DamageSource.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/Explosion.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

static DamageSource* NewDamageSource(ClientboundChatPacket::EChatPacketMessage message,
                                     ClientboundChatPacket::EChatPacketMessage deathMessage = ClientboundChatPacket::eChatPacketMessage_Text)
{
    return new DamageSource(message, deathMessage);
}

static const DamageSource* CreateFireDamageSource(ClientboundChatPacket::EChatPacketMessage message,
                                                  ClientboundChatPacket::EChatPacketMessage deathMessage)
{
    DamageSource* source = NewDamageSource(message, deathMessage);
    source->setIsFire();
    return source;
}

static const DamageSource* CreateLightningBoltDamageSource()
{
    DamageSource* source = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathLightningBolt, ClientboundChatPacket::eChatPacketMessage_DeathLightningBoltAlternate);
    source->bypassGraceInvul();
    return source;
}

static const DamageSource* CreateOnFireDamageSource()
{
    DamageSource* source = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOnFire, ClientboundChatPacket::eChatPacketMessage_DeathOnFireFighting);
    source->bypassArmor();
    source->setIsFire();
    return source;
}

static const DamageSource* CreateBypassArmorDamageSource(ClientboundChatPacket::EChatPacketMessage message,
                                                         ClientboundChatPacket::EChatPacketMessage deathMessage = ClientboundChatPacket::eChatPacketMessage_Text)
{
    DamageSource* source = NewDamageSource(message, deathMessage);
    source->bypassArmor();
    return source;
}

static const DamageSource* CreateStarveDamageSource()
{
    DamageSource* source = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathStarve);
    source->bypassArmor();
    source->bypassMagic();
    return source;
}

static const DamageSource* CreateBypassInvulDamageSource(ClientboundChatPacket::EChatPacketMessage message)
{
    DamageSource* source = NewDamageSource(message);
    source->bypassArmor();
    source->bypassInvul();
    return source;
}

static const DamageSource* CreateMagicDamageSource()
{
    DamageSource* source = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMagic);
    source->bypassArmor();
    source->setMagic();
    return source;
}

static const DamageSource* CreateFireworksDamageSource()
{
    DamageSource* source = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionAlternate);
    source->setExplosion();
    return source;
}

const DamageSource* DamageSource::IN_FIRE = CreateFireDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathInFire, ClientboundChatPacket::eChatPacketMessage_DeathInFireFighting);
const DamageSource* DamageSource::LIGHTNING_BOLT = CreateLightningBoltDamageSource();
const DamageSource* DamageSource::ON_FIRE = CreateOnFireDamageSource();
const DamageSource* DamageSource::LAVA = CreateFireDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathLava, ClientboundChatPacket::eChatPacketMessage_DeathLavaEscaping);
const DamageSource* DamageSource::HOT_FLOOR = CreateFireDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathHotFloor, ClientboundChatPacket::eChatPacketMessage_DeathHotFloorEscaping);
const DamageSource* DamageSource::IN_WALL = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathInWall);
const DamageSource* DamageSource::CRAMMING = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathCramming);
const DamageSource* DamageSource::DROWN = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathDrown, ClientboundChatPacket::eChatPacketMessage_DeathDrownEscaping);
const DamageSource* DamageSource::STARVE = CreateStarveDamageSource();
const DamageSource* DamageSource::CACTUS = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathCactus, ClientboundChatPacket::eChatPacketMessage_DeathCactusEscaping);
const DamageSource* DamageSource::FALL = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFall);
const DamageSource* DamageSource::FLY_INTO_WALL = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFlyIntoWall);
const DamageSource* DamageSource::OUT_OF_WORLD = CreateBypassInvulDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOutOfWorld);
const DamageSource* DamageSource::CREATIVE_PLAYER = CreateBypassInvulDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathCustom);
const DamageSource* DamageSource::GENERIC = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathGeneric);
const DamageSource* DamageSource::MAGIC = CreateMagicDamageSource();
const DamageSource* DamageSource::DRAGON_BREATH = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathDragonBreath);
const DamageSource* DamageSource::WITHER = CreateBypassArmorDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathWither);
const DamageSource* DamageSource::ANVIL = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathAnvil);
const DamageSource* DamageSource::FALLING_BLOCK = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFallingBlock);
const DamageSource* DamageSource::FIREWORKS = CreateFireworksDamageSource();
const DamageSource* DamageSource::ARROW = DamageSource::CreateArrow(boost::shared_ptr<Arrow>(), boost::shared_ptr<Entity>(), false);
const DamageSource* DamageSource::THORNS = DamageSource::CreateThorns(boost::shared_ptr<Entity>());
const DamageSource* DamageSource::POISON = DamageSource::MAGIC->copy();
const DamageSource* DamageSource::INSTANT_DAMAGE = DamageSource::MAGIC->copy();
const DamageSource* DamageSource::DRY_OUT = NewDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathDryOut);

DamageSource::DamageSource(ClientboundChatPacket::EChatPacketMessage messageValue, ClientboundChatPacket::EChatPacketMessage deathMessageValue)
    : message(messageValue), deathMessage(deathMessageValue), bypassArmorFlag(false), bypassInvulFlag(false), bypassGraceInvulFlag(false), bypassMagicFlag(false), foodExhaustion(FoodConstants::ExhaustionAttack()), fire(false), projectile(false), scalesDifficulty(false), magic(false), explosion(false), fireProjectile(false)
{
}

DamageSource::DamageSource(const DamageSource& source)
    : message(source.message), deathMessage(source.deathMessage), bypassArmorFlag(source.bypassArmorFlag), bypassInvulFlag(source.bypassInvulFlag), bypassGraceInvulFlag(source.bypassGraceInvulFlag), bypassMagicFlag(source.bypassMagicFlag), foodExhaustion(source.foodExhaustion), fire(source.fire), projectile(source.projectile), scalesDifficulty(source.scalesDifficulty), magic(source.magic), explosion(source.explosion), fireProjectile(source.fireProjectile)
{
}

bool DamageSource::equals(const DamageSource* source) const { return message == source->message && deathMessage == source->deathMessage; }

EntityDamageSource* DamageSource::CreateMobAttack(boost::shared_ptr<LivingEntity> entity)
{
    return new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMob, ClientboundChatPacket::eChatPacketMessage_DeathMob, entity);
}

void DamageSource::setProjectile() { projectile = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateIndirectMobAttack(const boost::shared_ptr<Entity>& source, const boost::shared_ptr<LivingEntity>& target, bool projectileValue)
{
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathMob, ClientboundChatPacket::eChatPacketMessage_DeathMob, source, target, 0));
    if (projectileValue) result->setProjectile();
    return result;
}

EntityDamageSource* DamageSource::CreatePlayerAttack(boost::shared_ptr<Player> player)
{
    return new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathPlayer, ClientboundChatPacket::eChatPacketMessage_DeathMobItem, player);
}

void DamageSource::setIsFireProjectile() { fireProjectile = true; }

DamageSource* DamageSource::CreateArrow(boost::shared_ptr<Arrow> arrow, boost::shared_ptr<Entity> owner, bool fireValue)
{
    boost::shared_ptr<Entity> arrowEntity(arrow, (Entity*)arrow.get());
    DamageSource* result = new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathArrow, ClientboundChatPacket::eChatPacketMessage_DeathArrowItem, arrowEntity, owner, 0);
    result->setProjectile();
    if (fireValue) result->setIsFireProjectile();
    return result;
}

boost::shared_ptr<DamageSource> DamageSource::CreateTrident(boost::shared_ptr<ThrownTrident> trident, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<Entity> tridentEntity(trident, (Entity*)trident.get());
    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathTrident, ClientboundChatPacket::eChatPacketMessage_DeathTridentItem, tridentEntity, owner, 0));
    result->setProjectile();
    return result;
}

void DamageSource::setIsFire() { fire = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateFireball(boost::shared_ptr<Fireball> fireball, boost::shared_ptr<Entity> owner)
{
    boost::shared_ptr<Entity> fireballEntity(fireball, (Entity*)fireball.get());
    boost::shared_ptr<DamageSource> result;
    if (owner)
        result.reset(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathFireball, ClientboundChatPacket::eChatPacketMessage_DeathArrowItem, fireballEntity, owner, 0));
    else
        result.reset(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathOnFire, ClientboundChatPacket::eChatPacketMessage_DeathOnFire, fireballEntity, fireballEntity, 0));
    result->setIsFire();
    result->setProjectile();
    return result;
}

DamageSource* DamageSource::CreateThrown(boost::shared_ptr<Entity> projectileEntity, boost::shared_ptr<Entity> owner)
{
    DamageSource* result = new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathThrown, ClientboundChatPacket::eChatPacketMessage_DeathThrownItem, projectileEntity, owner, 0);
    result->setProjectile();
    return result;
}

void DamageSource::setMagic() { magic = true; }
void DamageSource::bypassArmor() { bypassArmorFlag = true; foodExhaustion = 0.0f; }

boost::shared_ptr<DamageSource> DamageSource::CreateIndirectMagic(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> target, const DamageSource* baseSource)
{
    if (source->GetType() == EntityType::LLAMA_SPIT)
    {
        boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathPlayer, ClientboundChatPacket::eChatPacketMessage_DeathMobItem, source, target, baseSource));
        result->bypassArmor();
        result->setMagic();
        result->setProjectile();
        return result;
    }

    boost::shared_ptr<DamageSource> result(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagic, ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagicItem, source, target, baseSource));
    result->bypassArmor();
    result->setMagic();
    return result;
}

EntityDamageSource* DamageSource::CreateThorns(boost::shared_ptr<Entity> entity)
{
    EntityDamageSource* result = new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathThorns, ClientboundChatPacket::eChatPacketMessage_DeathThorns, entity);
    result->setThorns();
    result->setMagic();
    return result;
}

void DamageSource::setExplosion() { explosion = true; }
void DamageSource::setScalesWithDifficulty() { scalesDifficulty = true; }

boost::shared_ptr<DamageSource> DamageSource::CreateExplosion(Explosion* explosionValue)
{
    if (explosionValue == 0 || !explosionValue->getSourceMob())
    {
        boost::shared_ptr<DamageSource> result(new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosion, ClientboundChatPacket::eChatPacketMessage_Text));
        result->setExplosion();
        result->setScalesWithDifficulty();
        return result;
    }

    boost::shared_ptr<LivingEntity> sourceMob = explosionValue->getSourceMob();
    boost::shared_ptr<Entity> directSource = explosionValue->getDirectSource();
    if (directSource && directSource != sourceMob)
    {
        boost::shared_ptr<EntityDamageSource> result(new IndirectEntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, directSource, sourceMob, 0));
        result->setExplosion();
        result->setScalesWithDifficulty();
        return result;
    }

    boost::shared_ptr<EntityDamageSource> result(new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, sourceMob));
    result->setExplosion();
    result->setScalesWithDifficulty();
    return result;
}

boost::shared_ptr<DamageSource> DamageSource::CreateExplosion(const boost::shared_ptr<LivingEntity>& entity)
{
    if (!entity)
    {
        boost::shared_ptr<DamageSource> result(new DamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosion, ClientboundChatPacket::eChatPacketMessage_Text));
        result->setExplosion();
        result->setScalesWithDifficulty();
        return result;
    }

    boost::shared_ptr<DamageSource> result(new EntityDamageSource(ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy, entity));
    result->setExplosion();
    result->setScalesWithDifficulty();
    return result;
}

boost::shared_ptr<Entity> DamageSource::getDirectEntity() const { return getEntity(); }

boost::shared_ptr<Entity> DamageSource::getEntity() const
{
    return boost::shared_ptr<Entity>();
}
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
        std::wstring entityName = entity->getNetworkName();
        std::wstring killerName = killer->getNetworkName();
        ClientboundChatPacket::EChatPacketMessage packetMessage = deathMessage;
        if (packetMessage == ClientboundChatPacket::eChatPacketMessage_Text)
            packetMessage = message;
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(entityName, packetMessage, killer, killerName));
    }

    std::wstring entityName = entity->getNetworkName();
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
const DamageSource* DamageSource::GetBaseDamageSource() const { return this; }
DamageSource* DamageSource::copy() const { return new DamageSource(*this); }
void DamageSource::bypassInvul() { bypassInvulFlag = true; }
void DamageSource::bypassGraceInvul() { bypassGraceInvulFlag = true; }
void DamageSource::bypassMagic() { bypassMagicFlag = true; foodExhaustion = 0.0f; }
