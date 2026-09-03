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
    std::vector<MerchantRecipe*>::clear();
}

void MerchantRecipeList::load(CompoundTag* tag)
{
    if (tag == NULL)
    {
        return;
    }
    ListTag* recipes = tag->getList(L"Recipes", 10);
    if (recipes == NULL)
    {
        return;
    }
    for (int i = 0; i < recipes->size(); ++i)
    {
        push_back(new MerchantRecipe(recipes->getCompound(i)));
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
    if (count == 0)
    {
        return NULL;
    }
    unsigned int start = hint > 0 && static_cast<unsigned int>(hint) < count ? static_cast<unsigned int>(hint) : 0;
    for (unsigned int pass = 0; pass < count; ++pass)
    {
        unsigned int index = (start + pass) % count;
        MerchantRecipe* recipe = std::vector<MerchantRecipe*>::at(index);
        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        if (!ItemInstance::isSame(first, buyA) || first->getCount() < buyA->getCount())
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
        if (ItemInstance::isSame(second, buyB) && second->getCount() >= buyB->getCount())
        {
            return recipe;
        }
    }
    return NULL;
}

bool MerchantRecipeList::addIfNewOrBetter(MerchantRecipe* recipe)
{
    for (iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        MerchantRecipe* current = *it;
        if (current->isSame(recipe))
        {
            if (current->isSameSameButBetter(recipe))
            {
                delete current;
                *it = recipe;
                return true;
            }
            return false;
        }
    }
    push_back(recipe);
    return true;
}

MerchantRecipe* MerchantRecipeList::getMatchingRecipeFor(not_null_ptr<ItemInstance> first,
                                                         not_null_ptr<ItemInstance> second,
                                                         not_null_ptr<ItemInstance> result)
{
    for (iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        MerchantRecipe* recipe = *it;
        not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
        not_null_ptr<ItemInstance> sell = recipe->getSellItem();
        if (!ItemInstance::isSame(first, buyA) || first->getCount() < buyA->getCount() ||
            !ItemInstance::isSame(result, sell))
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
        if (ItemInstance::isSame(second, buyB) && second->getCount() >= buyB->getCount())
        {
            return recipe;
        }
    }
    return NULL;
}

void MerchantRecipeList::writeToStream(DataOutputStream* stream)
{
    stream->writeByte(static_cast<unsigned char>(std::vector<MerchantRecipe*>::size()));
    for (const_iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        MerchantRecipe* recipe = *it;
        stream->writeItem(recipe->getBuyAItem());
        stream->writeItem(recipe->getSellItem());
        not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();
        bool hasSecondary = !buyB->isEmpty();
        stream->writeBoolean(hasSecondary);
        if (hasSecondary)
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
    int count = stream->readUnsignedByte();
    for (int i = 0; i < count; ++i)
    {
        not_null_ptr<ItemInstance> buyA = stream->readItem();
        not_null_ptr<ItemInstance> sell = stream->readItem();
        not_null_ptr<ItemInstance> buyB;
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
    ListTag* recipes = new ListTag(0);
    for (const_iterator it = std::vector<MerchantRecipe*>::begin(); it != std::vector<MerchantRecipe*>::end(); ++it)
    {
        recipes->add((*it)->createTag());
    }
    tag->put(L"Recipes", recipes);
    return tag;
}

MerchantRecipe*& MerchantRecipeList::at(unsigned int index)
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
