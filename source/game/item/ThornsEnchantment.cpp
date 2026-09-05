#include "game/item/ThornsEnchantment.hpp"

#include "game/core/DamageSource.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

ThornsEnchantment::ThornsEnchantment(
    const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR, slots)
{
    setDescriptionId((int)0x88507d2e);
}

int ThornsEnchantment::getMinCost(int level)
{
    return level * 20 - 10;
}

int ThornsEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int ThornsEnchantment::getMaxLevel()
{
    return 3;
}

bool ThornsEnchantment::canEnchant(not_null_ptr<ItemInstance> item)
{
    if (dynamic_cast<ArmorItem*>(item->getItem()) != NULL)
    {
        return true;
    }

    return Enchantment::canEnchant(item);
}

bool ThornsEnchantment::shouldHit(int level, Random* random)
{
    if (level > 0)
    {
        return random->nextFloat() < (float)level * 0.15f;
    }

    return false;
}

int ThornsEnchantment::getDamage(int level, Random* random)
{
    if (level > 10)
    {
        return level - 10;
    }

    return random->nextInt(4) + 1;
}

void ThornsEnchantment::doPostHurt(boost::shared_ptr<LivingEntity> target,
                                   boost::shared_ptr<Entity> attacker,
                                   int level)
{
    Random* random = target->getRandom();
    boost::shared_ptr<ItemInstance> item =
        EnchantmentHelper::getRandomItemWith(this, target);

    if (item->isEmpty())
    {
        return;
    }

    if (!shouldHit(level, random))
    {
        item->hurtAndBreak(1, target);
    }
    else if (attacker)
    {
        boost::shared_ptr<EntityDamageSource> damageSource =
            DamageSource::CreateThorns(attacker);
        attacker->hurt(damageSource.get(), (float)getDamage(level, random));
        item->hurtAndBreak(3, target);
    }
    else
    {
        item->hurtAndBreak(3, target);
    }
}

ThornsEnchantment::~ThornsEnchantment()
{
}
