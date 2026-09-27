#include "game/entity/ai/MobCanWearArmourEntitySelector.hpp"
#include "game/core/EntityType.hpp"

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
    if ((type & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
    not_null_ptr<ItemInstance> equippedItem = livingEntity->getItemSlot(Mob::getEquipmentSlotForItem(mItemInstance));
    if (!equippedItem->isEmpty())
    {
        return false;
    }

    if (type == EntityType::VILLAGER)
    {
        return false;
    }

    if ((type & EntityType::ZOMBIE) == EntityType::ZOMBIE ||
        (type & EntityType::MONSTER) == EntityType::MONSTER)
    {
        return true;
    }

    if ((type & EntityType::MOB) == EntityType::MOB)
    {
        return boost::static_pointer_cast<Mob>(livingEntity)->canPickUpLoot();
    }

    if (type == EntityType::ARMOR_STAND)
    {
        return true;
    }

    return (type & EntityType::PLAYER) == EntityType::PLAYER;
}

MobCanWearArmourEntitySelector::~MobCanWearArmourEntitySelector()
{
}
