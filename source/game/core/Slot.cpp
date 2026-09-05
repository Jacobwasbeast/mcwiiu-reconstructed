#include "game/core/Slot.hpp"

#include "game/item/ArmorDyeRecipe.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/Container.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipes.hpp"

Slot::Slot(boost::shared_ptr<Container> container, int slot, int x, int y)
    : slot(slot)
    , container(container)
    , x(x)
    , y(y)
{
    index = 0;
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> oldItemInstance, not_null_ptr<ItemInstance> newItemInstance)
{
    int count = newItemInstance->getCount() - oldItemInstance->getCount();
    if (count > 0)
    {
        onQuickCraft(newItemInstance, count);
    }
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> itemInstance, int count)
{
}

void Slot::onSwapCraft(int count)
{
}

void Slot::checkTakeAchievements(not_null_ptr<ItemInstance> itemInstance)
{
}

void Slot::swap(Slot* other)
{
    not_null_ptr<ItemInstance> itemInstance = container->getItem(slot);
    not_null_ptr<ItemInstance> otherItemInstance = other->container->getItem(other->slot);

    if (!itemInstance->isEmpty() && itemInstance->getCount() > other->getMaxStackSize(itemInstance))
    {
        if (!otherItemInstance->isEmpty())
        {
            return;
        }

        otherItemInstance = itemInstance->split(itemInstance->getCount() - other->getMaxStackSize(itemInstance));
    }

    if (!otherItemInstance->isEmpty() && otherItemInstance->getCount() > getMaxStackSize(otherItemInstance))
    {
        if (!itemInstance->isEmpty())
        {
            return;
        }

        itemInstance = otherItemInstance->split(otherItemInstance->getCount() - getMaxStackSize(otherItemInstance));
    }

    other->container->setItem(other->slot, itemInstance);
    container->setItem(slot, otherItemInstance);
    setChanged();
}

not_null_ptr<ItemInstance> Slot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    setChanged();
    return itemInstance;
}

bool Slot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

not_null_ptr<ItemInstance> Slot::getItem()
{
    return container->getItem(slot);
}

bool Slot::hasItem()
{
    return !getItem()->isEmpty();
}

void Slot::set(not_null_ptr<ItemInstance> itemInstance)
{
    container->setItem(slot, itemInstance);
    setChanged();
}

void Slot::setChanged()
{
    container->setChanged();
}

int Slot::getMaxStackSize()
{
    return container->getMaxStackSize();
}

int Slot::getMaxStackSize(not_null_ptr<ItemInstance> itemInstance)
{
    return getMaxStackSize();
}

std::wstring Slot::getNoItemIcon()
{
    return std::wstring();
}

not_null_ptr<ItemInstance> Slot::remove(int count)
{
    return container->removeItem(slot, count);
}

bool Slot::isAt(boost::shared_ptr<Container> container, int slot)
{
    return container == this->container && slot == this->slot;
}

bool Slot::mayPickup(boost::shared_ptr<Player> player)
{
    return true;
}

bool Slot::isActive()
{
    return true;
}

bool Slot::mayCombine(not_null_ptr<ItemInstance> itemInstance)
{
    not_null_ptr<ItemInstance> currentItemInstance = getItem();

    if (!currentItemInstance->isEmpty() && !itemInstance->isEmpty())
    {
        if (!currentItemInstance->isEmpty() && !itemInstance->isEmpty()
            && Item::getId(currentItemInstance->getItem()) == Item::getId(itemInstance->getItem())
            && currentItemInstance->getCount() == 1 && itemInstance->getCount() == 1
            && currentItemInstance->getItem()->canBeDepleted()
            && (currentItemInstance->isDamaged() || itemInstance->isDamaged()))
        {
            return !currentItemInstance->isEnchanted() && !itemInstance->isEnchanted();
        }
    }

    return false;
}

not_null_ptr<ItemInstance> Slot::combine(not_null_ptr<ItemInstance> itemInstance)
{
    not_null_ptr<ItemInstance> currentItemInstance = getItem();
    boost::shared_ptr<CraftingContainer> craftingContainer(new CraftingContainer(NULL, 2, 2));

    craftingContainer->setItem(0, itemInstance);
    craftingContainer->setItem(1, currentItemInstance);

    not_null_ptr<ItemInstance> resultItemInstance = ItemInstance::EMPTY;

    if (dynamic_cast<ArmorItem*>(currentItemInstance->getItem()) != NULL)
    {
        resultItemInstance = ArmorDyeRecipe::assembleDyedArmor(craftingContainer);
    }
    else
    {
        resultItemInstance = Recipes::getInstance()->getItemFor(craftingContainer, NULL, NULL,
                                                               (Recipes::ERecipeCategory)0);
    }

    craftingContainer->setItem(0, ItemInstance::EMPTY);
    craftingContainer->setItem(1, ItemInstance::EMPTY);

    return resultItemInstance;
}

Slot::~Slot()
{
}
