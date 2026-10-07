#include "game/entity/ai/MobCanWearArmourEntitySelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"

MobCanWearArmourEntitySelector::MobCanWearArmourEntitySelector(not_null_ptr<ItemInstance> itemInstance)
{
    mItemInstance = itemInstance;
}

bool MobCanWearArmourEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity->isAlive())
    {
        return false;
    }

    if ((entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);
    if (!livingEntity->getItemSlot(Mob::getEquipmentSlotForItem(mItemInstance))->isEmpty())
    {
        return false;
    }

    const bool isVillager = livingEntity->GetType() == EntityType::VILLAGER;
    if (isVillager)
    {
        return false;
    }

    if ((livingEntity->GetType() & EntityType::ZOMBIE) == EntityType::ZOMBIE ||
        (livingEntity->GetType() & EntityType::ABSTRACT_SKELETON) == EntityType::ABSTRACT_SKELETON)
    {
        return true;
    }

    if ((livingEntity->GetType() & EntityType::MOB) == EntityType::MOB)
    {
        return boost::static_pointer_cast<Mob>(livingEntity)->canPickUpLoot();
    }

    const bool isArmorStand = livingEntity->GetType() == EntityType::ARMOR_STAND;
    if (isArmorStand)
    {
        return true;
    }

    if ((livingEntity->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
    {
        return false;
    }

    return true;
}

