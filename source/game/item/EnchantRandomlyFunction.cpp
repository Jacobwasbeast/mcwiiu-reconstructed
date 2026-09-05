#include "game/item/EnchantRandomlyFunction.hpp"

#include <boost/unordered_map.hpp>

#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/MappedRegistry.hpp"

EnchantRandomlyFunction::EnchantRandomlyFunction(std::vector<LootItemCondition*>* conditions,
                                                 const std::vector<Enchantment*>& enchantments)
    : LootItemFunction(conditions)
{
    this->enchantments = enchantments;
}

not_null_ptr<ItemInstance> EnchantRandomlyFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                          Random* random,
                                                          LootTableContext& context)
{
    not_null_ptr<ItemInstance> result = itemInstance;

    if (result->isEmpty())
    {
        return result;
    }

    if (enchantments.empty())
    {
        const boost::unordered_map<ResourceLocation,
                                   Enchantment*,
                                   boost::hash<ResourceLocation>,
                                   std::equal_to<ResourceLocation> >& registry =
            Enchantment::enchantmentRegistry->keySet();

        for (boost::unordered_map<ResourceLocation,
                                  Enchantment*,
                                  boost::hash<ResourceLocation>,
                                  std::equal_to<ResourceLocation> >::const_iterator it = registry.begin();
             it != registry.end();
             ++it)
        {
            if (it->second != NULL && it->second->canEnchant(result))
            {
                possibleEnchantments.push_back(it->second);
            }
        }
    }

    std::vector<Enchantment*>& choices = enchantments.empty() ? possibleEnchantments : enchantments;
    if (choices.empty())
    {
        return result;
    }

    Enchantment* enchantment = choices.at(random->nextInt((int)choices.size()));
    int level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());

    if (result->getItem() == Items::book)
    {
        EnchantmentInstance* instance = new EnchantmentInstance(enchantment, level);
        result = not_null_ptr<ItemInstance>(new ItemInstance(Items::enchantedBook));
        EnchantedBookItem::addEnchantment(result, instance);
    }
    else
    {
        result->enchant(enchantment, level);
    }

    return result;
}
