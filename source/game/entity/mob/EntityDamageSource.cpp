#include "game/entity/mob/EntityDamageSource.hpp"

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

EntityDamageSource::~EntityDamageSource()
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
        if ((type & 0x0c000000ULL) == 0x0c000000ULL)
            item = boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem();

        if (type == 0x0c800001ULL)
            attackerName = entity->getDisplayName();
        else if ((type & 0x0d000000ULL) == 0x0d000000ULL && entity->hasCustomName())
            attackerName = entity->getCustomName();
    }

    std::wstring victimName = victim->getDisplayName();
    if (item->hasCustomHoverName())
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, deathMessage, entity, attackerName, item->getHoverName()));
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, message, entity, attackerName));
}

bool EntityDamageSource::scalesWithDifficulty() const
{
    if (entity && (entity->GetType() & 0x0c000000ULL) == 0x0c000000ULL)
        return (entity->GetType() & 0x0c800000ULL) != 0x0c800000ULL;
    return false;
}

Vec3* EntityDamageSource::getSourcePosition() const
{
    return Vec3::newTemp(entity->mX, entity->mY, entity->mZ);
}

boost::shared_ptr<DamageSource> EntityDamageSource::copy() const
{
    return boost::shared_ptr<DamageSource>(new EntityDamageSource(*this));
}

void EntityDamageSource::setThorns()
{
    thorns = true;
}
