#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/world/generation/Player.hpp"

IndirectEntityDamageSource::IndirectEntityDamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                                                       ClientboundChatPacket::EChatPacketMessage deathMessageValue,
                                                       boost::shared_ptr<Entity> directEntityValue,
                                                       boost::shared_ptr<Entity> entityValue,
                                                       const DamageSource* baseDamageSourceValue)
    : EntityDamageSource(messageValue, deathMessageValue, directEntityValue),
      owner(entityValue),
      baseDamageSource(baseDamageSourceValue)
{
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity() const
{
    return entity;
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getEntity() const
{
    return owner;
}

boost::shared_ptr<ClientboundChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(boost::shared_ptr<LivingEntity> victim) const
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    std::wstring attackerName;

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        item = boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem();

    boost::shared_ptr<Entity> attacker = owner ? owner : entity;
    if (owner && owner->GetType() == EntityType::SERVER_PLAYER)
        attackerName = boost::static_pointer_cast<Player>(owner)->getDisplayName();

    std::wstring victimName = victim->getNetworkName();
    if (!item->isEmpty() && item->hasCustomHoverName())
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, deathMessage, attacker, attackerName, item->getHoverName()));
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, message, attacker, attackerName));
}

const DamageSource* IndirectEntityDamageSource::GetBaseDamageSource() const
{
    if (baseDamageSource)
        return baseDamageSource;
    return DamageSource::GetBaseDamageSource();
}

DamageSource* IndirectEntityDamageSource::copy() const
{
    return new IndirectEntityDamageSource(*this);
}
