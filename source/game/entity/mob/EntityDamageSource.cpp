#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/util/Vec3.hpp"

EntityDamageSource::EntityDamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                                       ClientboundChatPacket::EChatPacketMessage deathMessageValue,
                                       boost::shared_ptr<Entity> entityValue)
    : DamageSource(messageValue, deathMessageValue), entity(entityValue), thorns(false)
{
}

bool EntityDamageSource::isThorns() const
{
    return thorns;
}

boost::shared_ptr<Entity> EntityDamageSource::getEntity() const
{
    return entity;
}

boost::shared_ptr<ClientboundChatPacket> EntityDamageSource::getDeathMessagePacket(boost::shared_ptr<LivingEntity> victim) const
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    std::wstring attackerName;

    if (entity)
    {
        unsigned long long type = entity->GetType();
        if ((type & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
            item = boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem();

        if (type == EntityType::SERVER_PLAYER)
            attackerName = entity->getDisplayName();
        else if ((type & EntityType::MOB) == EntityType::MOB && entity->hasCustomName())
            attackerName = entity->getCustomName();
    }

    std::wstring victimName = victim->getNetworkName();
    if (item->hasCustomHoverName())
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, deathMessage, entity, attackerName, item->getHoverName()));
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, message, entity, attackerName));
}

bool EntityDamageSource::scalesWithDifficulty() const
{
    if (entity && (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        return (entity->GetType() & EntityType::PLAYER) != EntityType::PLAYER;
    return false;
}

Vec3* EntityDamageSource::getSourcePosition() const
{
    return Vec3::newTemp(entity->mX, entity->mY, entity->mZ);
}

DamageSource* EntityDamageSource::copy() const
{
    return new EntityDamageSource(*this);
}

void EntityDamageSource::setThorns()
{
    thorns = true;
}
