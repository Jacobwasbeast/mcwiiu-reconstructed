#include "game/item/FireworksRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/biome/arrayWithLength.hpp"

#include <vector>

unsigned int TlsAlloc();
int TlsSetValue(unsigned int index, void* pValue);
void* TlsGetValue(unsigned int index);

static const int DYE_COLORS[16] = {
    0x1E1B1B, 0xB3312C, 0x3B511A, 0x51301A,
    0x253192, 0x7B2FBE, 0x287697, 0xABABAB,
    0x434343, 0xD88198, 0x41CD34, 0xDECF2A,
    0x6689D3, 0xC354CD, 0xEB8844, 0xF0F0F0
};

static unsigned int g_fireworksRecipeTlsIndex = 0;
static FireworksRecipe::ThreadStorage* g_pDefaultThreadStorage = NULL;

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
    unsigned int slot;
    int gunpowder = 0;
    int charges = 0;
    int dyes = 0;
    int paper = 0;
    int effects = 0;
    int shapes = 0;

    *hasPaper = false;
    *hasGunpowder = false;
    *hasFireworkCharge = false;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->isEmpty())
        {
            continue;
        }

        if (item->getItem() == Items::gunpowder)
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
        else if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond)
        {
            ++effects;
        }
        else if (item->getItem() == Items::fireCharge || item->getItem() == Items::feather ||
                 item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
        {
            ++shapes;
        }
        else
        {
            return;
        }
    }

    if (gunpowder > 3 || paper > 1)
    {
        return;
    }

    if (effects + dyes + shapes == 0)
    {
        *hasPaper = true;
    }

    if (gunpowder > 1)
    {
        return;
    }

    if (paper == 0 && charges == 0 && shapes <= 1)
    {
        *hasGunpowder = true;
    }

    if (gunpowder != 0 || paper != 0 || charges > 1)
    {
        return;
    }

    *hasFireworkCharge = true;
}

bool FireworksRecipe::isValidIngredient(not_null_ptr<ItemInstance> item,
                                        bool hasPaper,
                                        bool hasGunpowder,
                                        bool hasFireworkCharge)
{
    if (item->getItem() == Items::gunpowder)
    {
        return hasPaper || hasGunpowder;
    }

    if (item->getItem() == Items::fireworkCharge)
    {
        return hasPaper || hasFireworkCharge;
    }

    if (item->getItem() == Items::dye)
    {
        return hasGunpowder || hasFireworkCharge;
    }

    if (item->getItem() == Items::paper)
    {
        return hasPaper;
    }

    if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond ||
        item->getItem() == Items::fireCharge || item->getItem() == Items::feather ||
        item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
    {
        return hasGunpowder;
    }

    return false;
}

void FireworksRecipe::setResultItem(not_null_ptr<ItemInstance> item)
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    pStorage->mResultItem = item;
}

void FireworksRecipe::CreateNewThreadStorage()
{
    ThreadStorage* pStorage = new ThreadStorage();

    if (g_pDefaultThreadStorage == NULL)
    {
        g_fireworksRecipeTlsIndex = TlsAlloc();
        g_pDefaultThreadStorage = pStorage;
    }

    TlsSetValue(g_fireworksRecipeTlsIndex, pStorage);
}

void FireworksRecipe::UseDefaultThreadStorage()
{
    TlsSetValue(g_fireworksRecipeTlsIndex, g_pDefaultThreadStorage);
}

void FireworksRecipe::ReleaseThreadStorage()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    if (pStorage != g_pDefaultThreadStorage)
    {
        delete pStorage;
    }
}

bool FireworksRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    unsigned int slot;
    int gunpowder = 0;
    int charges = 0;
    int dyes = 0;
    int paper = 0;
    int effects = 0;
    int shapes = 0;
    int extras;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->isEmpty())
        {
            continue;
        }

        if (item->getItem() == Items::gunpowder)
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
        else if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond)
        {
            ++effects;
        }
        else if (item->getItem() == Items::fireCharge || item->getItem() == Items::feather ||
                 item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
        {
            ++shapes;
        }
        else
        {
            setResultItem(ItemInstance::EMPTY);
            return false;
        }
    }

    extras = effects + dyes + shapes;

    if (gunpowder > 3 || paper > 1)
    {
        setResultItem(ItemInstance::EMPTY);
        return false;
    }

    if (gunpowder >= 1 && paper == 1 && extras == 0)
    {
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::fireworks, 3));
        CompoundTag* fireworks = new CompoundTag();

        if (charges > 0)
        {
            ListTag* explosions = new ListTag();

            for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
            {
                not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

                if (item->getItem() == Items::fireworkCharge && item->hasTag() &&
                    item->getTag()->contains(L"Explosion", 10))
                {
                    explosions->add(item->getTag()->getCompound(L"Explosion")->copy());
                }
            }

            fireworks->put(L"Explosions", explosions);
        }

        fireworks->putByte(L"Flight", static_cast<unsigned char>(gunpowder));

        CompoundTag* tag = new CompoundTag();
        tag->put(L"Fireworks", fireworks);
        result->setTag(tag);
        setResultItem(result);
        return true;
    }

    if (gunpowder == 1 && paper == 0 && charges == 0 && dyes >= 1 && shapes <= 1)
    {
        not_null_ptr<ItemInstance> result(new ItemInstance(Items::fireworkCharge));
        std::vector<int> colors;
        CompoundTag* tag = new CompoundTag();
        CompoundTag* explosion = new CompoundTag();
        unsigned char shape = 0;

        for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

            if (item->isEmpty())
            {
                continue;
            }

            if (item->getItem() == Items::dye)
            {
                colors.push_back(DYE_COLORS[item->getAuxValue() & 15]);
            }
            else if (item->getItem() == Items::glowstoneDust)
            {
                explosion->putBoolean(L"Flicker", true);
            }
            else if (item->getItem() == Items::diamond)
            {
                explosion->putBoolean(L"Trail", true);
            }
            else if (item->getItem() == Items::fireCharge)
            {
                shape = 1;
            }
            else if (item->getItem() == Items::feather)
            {
                shape = 4;
            }
            else if (item->getItem() == Items::goldNugget)
            {
                shape = 2;
            }
            else if (item->getItem() == Items::skull)
            {
                shape = 3;
            }
        }

        arrayWithLength<int> explosionColors(colors);
        explosion->putIntArray(L"Colors", explosionColors);
        explosion->putByte(L"Type", shape);
        tag->put(L"Explosion", explosion);
        result->setTag(tag);
        setResultItem(result);
        return true;
    }

    if (gunpowder == 0 && paper == 0 && charges == 1 && dyes >= 1 && dyes == extras)
    {
        not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
        std::vector<int> colors;

        for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

            if (item->isEmpty())
            {
                continue;
            }

            if (item->getItem() == Items::dye)
            {
                colors.push_back(DYE_COLORS[item->getAuxValue() & 15]);
            }
            else if (item->getItem() == Items::fireworkCharge)
            {
                result = item->copy();
                result->setCount(1);
            }
        }

        arrayWithLength<int> fadeColors(colors);

        if (!result->isEmpty() && result->hasTag())
        {
            CompoundTag* explosion = result->getTag()->getCompound(L"Explosion");

            if (explosion != NULL)
            {
                explosion->putIntArray(L"FadeColors", fadeColors);
                setResultItem(result);
                return true;
            }
        }
    }

    setResultItem(ItemInstance::EMPTY);
    return false;
}

not_null_ptr<ItemInstance> FireworksRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    return pStorage->mResultItem->copy();
}

int FireworksRecipe::size()
{
    return 10;
}

ItemInstance* FireworksRecipe::getResultItem()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    return pStorage->mResultItem.get();
}

ItemInstance* FireworksRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > FireworksRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int slot;
    NonNullList<not_null_ptr<ItemInstance> > remaining =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);

    for (slot = 0; slot < remaining.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->getItem()->hasCraftingRemainingItem())
        {
            remaining[slot] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }

    return remaining;
}

boost::shared_ptr<CraftingContainer> FireworksRecipe::generateExpectedCraftSlots()
{
    boost::shared_ptr<CraftingContainer> result;
    unsigned int* storage = reinterpret_cast<unsigned int*>(&result);
    if (storage != NULL || (storage = static_cast<unsigned int*>(operator new(8)), storage != NULL))
    {
        storage[0] = 0;
        storage[1] = 0;
    }
    return result;
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
