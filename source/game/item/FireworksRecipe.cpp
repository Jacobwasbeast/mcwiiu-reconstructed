#include "game/item/FireworksRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/DyeColor.hpp"
#include <vector>

int TlsAlloc();
void* TlsGetValue(int index);
void TlsSetValue(int index, void* value);

static int sThreadStorageTlsIndex = 0;
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
    int dyes = 0;
    int glowstone = 0;
    int diamonds = 0;
    int fireCharges = 0;
    int goldNuggets = 0;
    int feathers = 0;

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
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
        }
        else if (item->getItem() == Items::glowstoneDust)
        {
            ++glowstone;
        }
        else if (item->getItem() == Items::diamond)
        {
            ++diamonds;
        }
        else if (item->getItem() == Items::fireCharge)
        {
            ++fireCharges;
        }
        else if (item->getItem() == Items::goldNugget)
        {
            ++goldNuggets;
        }
        else if (item->getItem() == Items::feather)
        {
            ++feathers;
        }
    }

    *hasPaper = dyes == 0 && glowstone == 0 && diamonds == 0 && fireCharges == 0 &&
        goldNuggets == 0 && feathers == 0;
    *hasGunpowder = paper <= 1 && gunpowder == 0 && charges < 2 &&
        glowstone == 0 && diamonds == 0 && fireCharges == 0 && goldNuggets == 0 && feathers == 0;
    *hasFireworkCharge = paper == 0 && gunpowder == 0 && charges <= 1 &&
        dyes == 0 && glowstone == 0 && diamonds == 0 && fireCharges == 0 && goldNuggets == 0 && feathers == 0;
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
    int dyes = 0;
    int glowstone = 0;
    int diamonds = 0;
    int fireCharges = 0;
    int goldNuggets = 0;
    int feathers = 0;
    std::vector<int> colors;
    unsigned char type = 0;

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
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
            switch (item->getAuxValue() & 15)
            {
            case 0: colors.push_back(15790320); break;
            case 1: colors.push_back(14602026); break;
            case 2: colors.push_back(11743532); break;
            case 3: colors.push_back(15435844); break;
            case 4: colors.push_back(8073150); break;
            case 5: colors.push_back(14612849); break;
            case 6: colors.push_back(4312372); break;
            case 7: colors.push_back(4408131); break;
            case 8: colors.push_back(3887386); break;
            case 9: colors.push_back(14188952); break;
            case 10: colors.push_back(4312372); break;
            case 11: colors.push_back(6719955); break;
            case 12: colors.push_back(12801229); break;
            case 13: colors.push_back(15435844); break;
            case 14: colors.push_back(16701501); break;
            default: colors.push_back(1973019); break;
            }
        }
        else if (item->getItem() == Items::glowstoneDust)
        {
            ++glowstone;
        }
        else if (item->getItem() == Items::diamond)
        {
            ++diamonds;
        }
        else if (item->getItem() == Items::fireCharge)
        {
            ++fireCharges;
            type = 1;
        }
        else if (item->getItem() == Items::goldNugget)
        {
            ++goldNuggets;
            type = 2;
        }
        else if (item->getItem() == Items::feather)
        {
            ++feathers;
            type = 3;
        }
        else
        {
            return false;
        }
    }

    if (paper == 1 && gunpowder >= 1 && gunpowder <= 3 && charges <= 3 &&
        dyes == 0 && glowstone == 0 && diamonds == 0 && fireCharges == 0 &&
        goldNuggets == 0 && feathers == 0)
    {
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::fireworks, 1, 0));
        CompoundTag* tag = new CompoundTag();
        ListTag* explosions = new ListTag(10);
        tag->putByte(L"Flight", static_cast<unsigned char>(gunpowder));
        for (i = 0; i < craftingContainer->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
            if (item->isEmpty() || item->getItem() != Items::fireworkCharge)
            {
                continue;
            }
            if (item->hasTag())
            {
                CompoundTag* explosion = item->getTag()->getCompound(L"Explosion");
                if (explosion != NULL)
                {
                    explosions->add(explosion->copy());
                }
            }
        }
        tag->put(L"Explosions", explosions);
        result->setTag(tag);
        setResultItem(result);
        return true;
    }

    if (paper != 0 || gunpowder != 1 || charges != 0 || dyes < 1 ||
        glowstone > 1 || diamonds > 1 || fireCharges > 1 || goldNuggets > 1 || feathers > 1 ||
        glowstone + diamonds + fireCharges + goldNuggets + feathers > 1)
    {
        return false;
    }

    not_null_ptr<ItemInstance> result(new ItemInstance(Items::fireworkCharge, 1, 0));
    CompoundTag* tag = new CompoundTag();
    CompoundTag* explosion = new CompoundTag();
    arrayWithLength<int> colorArray(colors);
    explosion->putIntArray(L"Colors", colorArray);
    explosion->putByte(L"Type", type);
    if (glowstone != 0)
    {
        explosion->putBoolean(L"Flicker", true);
    }
    if (diamonds != 0)
    {
        explosion->putBoolean(L"Trail", true);
    }
    tag->put(L"Explosion", explosion);
    result->setTag(tag);
    setResultItem(result);
    return true;
}

not_null_ptr<ItemInstance> FireworksRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sThreadStorageTlsIndex));
    return storage->mResultItem->copy();
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
