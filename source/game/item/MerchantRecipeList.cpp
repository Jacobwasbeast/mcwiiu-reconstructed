#include "game/item/MerchantRecipeList.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"

MerchantRecipeList::MerchantRecipeList()
    : std::vector<MerchantRecipe*>()
{
}

MerchantRecipeList::MerchantRecipeList(CompoundTag* tag)
    : std::vector<MerchantRecipe*>()
{
    load(tag);
}

MerchantRecipeList::~MerchantRecipeList()
{
    for (iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        delete *it;
    }
}

void MerchantRecipeList::load(CompoundTag* tag)
{
    ListTag* recipes = tag->getList(L"Recipes", Tag::TAG_COMPOUND);
    for (int i = 0; i < recipes->size(); ++i)
    {
        std::vector<MerchantRecipe*>::push_back(new MerchantRecipe(recipes->getCompound(i)));
    }
}

bool MerchantRecipeList::isRequiredItem(not_null_ptr<ItemInstance> supplied,
                                        not_null_ptr<ItemInstance> required)
{
    return ItemInstance::isSame(supplied, required);
}

MerchantRecipe* MerchantRecipeList::getRecipeFor(not_null_ptr<ItemInstance> first,
                                                 not_null_ptr<ItemInstance> second,
                                                 int hint)
{
    unsigned int count = static_cast<unsigned int>(std::vector<MerchantRecipe*>::size());

    if (hint >= 1 && static_cast<unsigned int>(hint) < count)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(static_cast<unsigned int>(hint));
        bool matched = false;
        if (isRequiredItem(first, recipe->getBuyAItem()))
        {
            if (!second->isEmpty() && !recipe->hasSecondaryBuyItem())
            {
                matched = false;
            }
            else if (recipe->hasSecondaryBuyItem())
            {
                matched = isRequiredItem(second, recipe->getBuyAItem());
            }
            else
            {
                matched = true;
            }
        }
        if (!matched)
        {
            return NULL;
        }

        bool countOk = first->getCount() >= recipe->getBuyAItem()->getCount();
        if (countOk && recipe->hasSecondaryBuyItem())
        {
            countOk = second->getCount() >= recipe->getBuyBItem()->getCount();
        }
        if (!countOk)
        {
            return NULL;
        }
        return recipe;
    }

    for (unsigned int index = 0; index < count; ++index)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(index);
        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        if (!isRequiredItem(first, buyA) || first->getCount() < buyA->getCount())
        {
            continue;
        }
        if (!recipe->hasSecondaryBuyItem())
        {
            if (second->isEmpty())
            {
                return recipe;
            }
            continue;
        }
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        if (isRequiredItem(second, buyB) && second->getCount() >= buyB->getCount())
        {
            return recipe;
        }
    }
    return NULL;
}

bool MerchantRecipeList::addIfNewOrBetter(MerchantRecipe* recipe)
{
    for (unsigned int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(i);
        if (recipe->isSame(recipe))
        {
            bool replaced = false;
            if (recipe->isSameSameButBetter(recipe))
            {
                delete (*this)[i];
                (*this)[i] = recipe;
                replaced = true;
            }
            return replaced;
        }
    }
    std::vector<MerchantRecipe*>::push_back(recipe);
    return true;
}

MerchantRecipe* MerchantRecipeList::getMatchingRecipeFor(not_null_ptr<ItemInstance> first,
                                                         not_null_ptr<ItemInstance> second,
                                                         not_null_ptr<ItemInstance> result)
{
    unsigned int count = static_cast<unsigned int>(std::vector<MerchantRecipe*>::size());
    for (unsigned int index = 0; index < count; ++index)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(index);
        bool matched = false;
        if (first->getItem() == recipe->getBuyAItem()->getItem() &&
            first->getCount() >= recipe->getBuyAItem()->getCount())
        {
            matched = result->getItem() == recipe->getSellItem()->getItem();
        }

        if (!matched)
        {
            continue;
        }

        if (recipe->hasSecondaryBuyItem())
        {
            matched = !second->isEmpty() && second->getItem() == recipe->getBuyBItem()->getItem() &&
                      second->getCount() >= recipe->getBuyBItem()->getCount();
        }

        if (matched)
        {
            return recipe;
        }
    }
    return NULL;
}

void MerchantRecipeList::writeToStream(DataOutputStream* stream)
{
    stream->writeByte(static_cast<unsigned char>(std::vector<MerchantRecipe*>::size()));
    for (unsigned int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(i);
        stream->writeItem(recipe->getBuyAItem());
        stream->writeItem(recipe->getSellItem());
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        stream->writeBoolean(!buyB->isEmpty());
        if (!buyB->isEmpty())
        {
            stream->writeItem(buyB);
        }
        stream->writeBoolean(recipe->isDeprecated());
        stream->writeInt(recipe->getUses());
        stream->writeInt(recipe->getMaxUses());
    }
}

void MerchantRecipeList::push_back(MerchantRecipe* recipe)
{
    std::vector<MerchantRecipe*>::push_back(recipe);
}

MerchantRecipeList* MerchantRecipeList::createFromStream(DataInputStream* stream)
{
    MerchantRecipeList* result = new MerchantRecipeList();
    int count = stream->readByte();
    for (int i = 0; i < count; ++i)
    {
        not_null_ptr<ItemInstance> buyA = stream->readItem();
        not_null_ptr<ItemInstance> sell = stream->readItem();
        not_null_ptr<ItemInstance> buyB = ItemInstance::EMPTY;
        if (stream->readBoolean())
        {
            buyB = stream->readItem();
        }
        bool deprecated = stream->readBoolean();
        int uses = stream->readInt();
        int maxUses = stream->readInt();
        MerchantRecipe* recipe = new MerchantRecipe(buyA, buyB, sell, uses, maxUses);
        if (deprecated)
        {
            recipe->enforceDeprecated();
        }
        result->push_back(recipe);
    }
    return result;
}

CompoundTag* MerchantRecipeList::createTag()
{
    CompoundTag* tag = new CompoundTag();
    ListTag* recipes = new ListTag();
    for (unsigned int i = 0; i < std::vector<MerchantRecipe*>::size(); ++i)
    {
        recipes->add(std::vector<MerchantRecipe*>::at(i)->createTag());
    }
    tag->put(L"Recipes", recipes);
    return tag;
}

MerchantRecipe* MerchantRecipeList::at(unsigned int index)
{
    return std::vector<MerchantRecipe*>::at(index);
}

MerchantRecipeList::iterator MerchantRecipeList::begin()
{
    return std::vector<MerchantRecipe*>::begin();
}

MerchantRecipeList::iterator MerchantRecipeList::end()
{
    return std::vector<MerchantRecipe*>::end();
}

int MerchantRecipeList::size()
{
    return static_cast<int>(std::vector<MerchantRecipe*>::size());
}

bool MerchantRecipeList::empty()
{
    return std::vector<MerchantRecipe*>::empty();
}

MerchantRecipeList::iterator MerchantRecipeList::erase(iterator position)
{
    delete *position;
    return std::vector<MerchantRecipe*>::erase(position);
}
