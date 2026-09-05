#include "game/item/PotionBrewing.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"

static std::vector<PotionBrewing::Mix<Item>*> containerMixes;
static std::vector<PotionBrewing::Mix<Potion>*> potionMixes;
static std::vector<PotionBrewing::ItemInstancePredicate*> containerPredicates;

PotionBrewing::ItemInstancePredicate::ItemInstancePredicate(Item* item)
    : mItem(item), mAuxValue(-1)
{
}

PotionBrewing::ItemInstancePredicate::ItemInstancePredicate(Item* item, int auxValue)
    : mItem(item), mAuxValue(auxValue)
{
}

bool PotionBrewing::ItemInstancePredicate::apply(not_null_ptr<ItemInstance> itemInstance) const
{
    if (itemInstance->getItem() != mItem)
        return false;
    return mAuxValue == -1 || itemInstance->getAuxValue() == mAuxValue;
}

PotionBrewing::ItemInstancePredicate::~ItemInstancePredicate()
{
}

bool PotionBrewing::PredicateItemInstance::apply(not_null_ptr<ItemInstance> itemInstance) const
{
    std::vector<PotionBrewing::ItemInstancePredicate*>::const_iterator it = containerPredicates.begin();
    while (it != containerPredicates.end())
    {
        if ((*it)->apply(itemInstance))
            return true;
        ++it;
    }
    return false;
}

PotionBrewing::PredicateItemInstance::~PredicateItemInstance()
{
}

bool PotionBrewing::isContainerIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    std::vector<Mix<Item>*>::const_iterator it = containerMixes.begin();
    while (it != containerMixes.end())
    {
        if ((*it)->ingredient->apply(itemInstance))
            return true;
        ++it;
    }
    return false;
}

bool PotionBrewing::isPotionIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    std::vector<Mix<Potion>*>::const_iterator it = potionMixes.begin();
    while (it != potionMixes.end())
    {
        if ((*it)->ingredient->apply(itemInstance))
            return true;
        ++it;
    }
    return false;
}

bool PotionBrewing::isIngredient(not_null_ptr<ItemInstance> itemInstance)
{
    return isContainerIngredient(itemInstance) || isPotionIngredient(itemInstance);
}

bool PotionBrewing::hasContainerMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    Item* itemType = item->getItem();
    std::vector<Mix<Item>*>::const_iterator it = containerMixes.begin();
    while (it != containerMixes.end())
    {
        if ((*it)->from == itemType && (*it)->ingredient->apply(ingredient))
            return true;
        ++it;
    }
    return false;
}

bool PotionBrewing::hasPotionMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    Potion* potion = PotionUtils::getPotion(item);
    std::vector<Mix<Potion>*>::const_iterator it = potionMixes.begin();
    while (it != potionMixes.end())
    {
        if ((*it)->from == potion && (*it)->ingredient->apply(ingredient))
            return true;
        ++it;
    }
    return false;
}

bool PotionBrewing::hasMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    if (hasContainerMix(item, ingredient))
        return true;
    return hasPotionMix(item, ingredient);
}

void PotionBrewing::mix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient)
{
    if (!item->isEmpty())
    {
        Potion* potion = PotionUtils::getPotion(item);
        Item* itemType = item->getItem();
        std::vector<Mix<Item>*>::const_iterator itemIt = containerMixes.begin();
        while (itemIt != containerMixes.end())
        {
            if ((*itemIt)->from == itemType && (*itemIt)->ingredient->apply(ingredient))
            {
                not_null_ptr<ItemInstance> result(new ItemInstance((*itemIt)->result));
                PotionUtils::setPotion(item, potion);
                return;
            }
            ++itemIt;
        }
        std::vector<Mix<Potion>*>::const_iterator potionIt = potionMixes.begin();
        while (potionIt != potionMixes.end())
        {
            if ((*potionIt)->from == potion && (*potionIt)->ingredient->apply(ingredient))
            {
                PotionUtils::setPotion(item, (*potionIt)->result);
                return;
            }
            ++potionIt;
        }
    }
}

void PotionBrewing::addContainer(ItemInstancePredicate* predicate)
{
    containerPredicates.push_back(predicate);
}

void PotionBrewing::addContainerRecipe(PotionItem* from, ItemInstancePredicate* ingredient, PotionItem* result)
{
    Mix<Item>* mix = new Mix<Item>;
    mix->from = (Item*)from;
    mix->ingredient = ingredient;
    mix->result = (Item*)result;
    containerMixes.push_back(mix);
}

void PotionBrewing::addMix(Potion* from, Predicate<not_null_ptr<ItemInstance> >* ingredient, Potion* result)
{
    Mix<Potion>* mix = new Mix<Potion>;
    mix->from = from;
    mix->ingredient = ingredient;
    mix->result = result;
    potionMixes.push_back(mix);
}

void PotionBrewing::StaticCtor()
{
    ItemInstancePredicate* potion = new ItemInstancePredicate(Items::potion);
    ItemInstancePredicate* splashPotion = new ItemInstancePredicate(Items::splashPotion);
    ItemInstancePredicate* gunpowder = new ItemInstancePredicate(Items::gunpowder);
    ItemInstancePredicate* dragonBreath = new ItemInstancePredicate(Items::dragonBreath);
    addContainer(potion);
    addContainer(splashPotion);
    addContainer(new ItemInstancePredicate(Items::lingeringPotion));
    addContainerRecipe((PotionItem*)Items::potion, gunpowder, (PotionItem*)Items::splashPotion);
    addContainerRecipe((PotionItem*)Items::splashPotion, dragonBreath, (PotionItem*)Items::lingeringPotion);
    addMix(Potions::WATER, new ItemInstancePredicate(Items::netherWart), Potions::AWKWARD);
    addMix(Potions::AWKWARD, new ItemInstancePredicate(Items::sugar), Potions::SWIFTNESS);
}
