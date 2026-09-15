#include "game/entity/ai/MobCanWearArmourEntitySelector.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"

MobCanWearArmourEntitySelector::MobCanWearArmourEntitySelector(not_null_ptr<ItemInstance> itemInstance)
    : mItemInstance(itemInstance)
{
}

bool MobCanWearArmourEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity->isAlive())
    {
        return false;
    }

    const unsigned long long type = entity->GetType();
    if ((type & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
    not_null_ptr<ItemInstance> equippedItem = livingEntity->getItemSlot(Mob::getEquipmentSlotForItem(mItemInstance));
    if (!equippedItem->isEmpty())
    {
        return false;
    }

    if (type == ENTITY_TYPE_VILLAGER)
    {
        return false;
    }

    if ((type & ENTITY_TYPE_ZOMBIE) == ENTITY_TYPE_ZOMBIE ||
        (type & ENTITY_TYPE_MONSTER) == ENTITY_TYPE_MONSTER)
    {
        return true;
    }

    if ((type & ENTITY_TYPE_MOB) == ENTITY_TYPE_MOB)
    {
        return boost::static_pointer_cast<Mob>(livingEntity)->canPickUpLoot();
    }

    if (type == ENTITY_TYPE_ARMOR_STAND)
    {
        return true;
    }

    return (type & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER;
}

MobCanWearArmourEntitySelector::~MobCanWearArmourEntitySelector()
{
}
