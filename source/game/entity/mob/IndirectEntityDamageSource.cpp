#include "game/entity/mob/IndirectEntityDamageSource.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"

IndirectEntityDamageSource::IndirectEntityDamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                                                       ClientboundChatPacket::EChatPacketMessage deathMessageValue,
                                                       boost::shared_ptr<Entity> directEntityValue,
                                                       boost::shared_ptr<Entity> entityValue,
                                                       const DamageSource* baseDamageSourceValue)
    : EntityDamageSource(messageValue, deathMessageValue, directEntityValue),
      directEntity(entityValue),
      baseDamageSource(baseDamageSourceValue)
{
}

IndirectEntityDamageSource::~IndirectEntityDamageSource()
{
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity() const
{
    return entity;
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getEntity() const
{
    return directEntity;
}

boost::shared_ptr<ClientboundChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(boost::shared_ptr<LivingEntity> victim) const
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    std::wstring attackerName;

    if (entity && (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        item = boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem();

    boost::shared_ptr<Entity> attacker = directEntity ? directEntity : entity;
    if (directEntity && directEntity->GetType() == 0x0c800001ULL)
        attackerName = directEntity->getDisplayName();

    std::wstring victimName = victim->getDisplayName();
    if (item->hasCustomHoverName())
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, deathMessage, attacker, attackerName, item->getHoverName()));
    return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(victimName, message, attacker, attackerName));
}

const DamageSource* IndirectEntityDamageSource::GetBaseDamageSource() const
{
    if (baseDamageSource)
        return baseDamageSource;
    return DamageSource::GetBaseDamageSource();
}

boost::shared_ptr<DamageSource> IndirectEntityDamageSource::copy() const
{
    return boost::shared_ptr<DamageSource>(new IndirectEntityDamageSource(*this));
}
