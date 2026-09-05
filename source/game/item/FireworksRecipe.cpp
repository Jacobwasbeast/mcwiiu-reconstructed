#include "game/item/FireworksRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

unsigned int TlsAlloc();
void* TlsGetValue(unsigned int index);
void TlsSetValue(unsigned int index, void* value);

static unsigned int sThreadStorageTlsIndex = 0;
static FireworksRecipe::ThreadStorage* sDefaultThreadStorage = NULL;

FireworksRecipe::ThreadStorage::ThreadStorage()
    : mResultItem(ItemInstance::EMPTY)
{
}

FireworksRecipe::FireworksRecipe()
    : Recipe()
{
}

FireworksRecipe::~FireworksRecipe()
{
}

void FireworksRecipe::updatePossibleRecipes(boost::shared_ptr<CraftingContainer> craftingContainer,
                                            bool* hasPaper,
                                            bool* hasGunpowder,
                                            bool* hasFireworkCharge)
{
    unsigned int i;
    int paper = 0;
    int gunpowder = 0;
    int charges = 0;

    *hasPaper = false;
    *hasGunpowder = false;
    *hasFireworkCharge = false;

    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::gunpowder)
        {
            ++gunpowder;
        }
        else if (item->getItem() == Items::fireworkCharge)
        {
            ++charges;
        }
    }

    *hasPaper = paper == 0 && gunpowder == 0 && charges == 0;
    *hasGunpowder = paper <= 1 && gunpowder == 0 && charges < 2;
    *hasFireworkCharge = paper == 0 && gunpowder == 0 && charges <= 1;
}

bool FireworksRecipe::isValidIngredient(not_null_ptr<ItemInstance> item,
                                        bool allowPaper,
                                        bool allowGunpowder,
                                        bool allowCharge)
{
    if (!item || item->isEmpty())
    {
        return false;
    }

    Item* type = item->getItem();
    if (type == Items::paper)
    {
        return allowPaper;
    }
    if (type == Items::gunpowder)
    {
        return allowGunpowder;
    }
    if (type == Items::fireworkCharge)
    {
        return allowCharge;
    }
    return type == Items::dye || type == Items::glowstoneDust || type == Items::diamond ||
        type == Items::fireCharge || type == Items::goldNugget || type == Items::feather;
}

void FireworksRecipe::setResultItem(not_null_ptr<ItemInstance> item)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));
    storage->mResultItem = item;
}

void FireworksRecipe::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    if (sDefaultThreadStorage == NULL)
    {
        sThreadStorageTlsIndex = TlsAlloc();
        sDefaultThreadStorage = storage;
    }
    TlsSetValue(sThreadStorageTlsIndex, storage);
}

void FireworksRecipe::UseDefaultThreadStorage()
{
    TlsSetValue(sThreadStorageTlsIndex, sDefaultThreadStorage);
}

void FireworksRecipe::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));
    if (storage != sDefaultThreadStorage)
    {
        delete storage;
    }
}

bool FireworksRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    unsigned int i;
    int paper = 0;
    int gunpowder = 0;
    int charges = 0;

    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::gunpowder)
        {
            ++gunpowder;
        }
        else if (item->getItem() == Items::fireworkCharge)
        {
            ++charges;
        }
        else
        {
            return false;
        }
    }

    if (paper != 1 || gunpowder < 1 || gunpowder > 3 || charges > 3)
    {
        return false;
    }

    not_null_ptr<ItemInstance> result(new ItemInstance(Items::fireworks, 1, 0));
    CompoundTag* tag = new CompoundTag();
    tag->putByte(L"Flight", static_cast<unsigned char>(gunpowder));
    tag->put(L"Explosions", new ListTag(10));
    result->setTag(tag);
    setResultItem(result);
    return true;
}

not_null_ptr<ItemInstance> FireworksRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));
    return storage->mResultItem->copy_not_shared();
}

int FireworksRecipe::size()
{
    return 10;
}

ItemInstance* FireworksRecipe::getResultItem()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));
    return storage->mResultItem.get();
}

ItemInstance* FireworksRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > FireworksRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem()->hasCraftingRemainingItem())
        {
            result[i] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }
    return result;
}

boost::shared_ptr<CraftingContainer> FireworksRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}

Recipe::_eGroupType FireworksRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool FireworksRecipe::recipeRequires(int itemId)
{
    return false;
}

void FireworksRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}
