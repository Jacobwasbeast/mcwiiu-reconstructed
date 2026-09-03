#include "game/item/MerchantRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

MerchantRecipe::MerchantRecipe(CompoundTag* tag)
    : mBuyA(), mBuyB(), mSell(), mUses(0), mMaxUses(12), mRewardExp(true), mDeprecated(false)
{
    load(tag);
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB,
                               not_null_ptr<ItemInstance> sell, int maxUses, int xp)
    : mBuyA(buyA), mBuyB(buyB), mSell(sell), mUses(0), mMaxUses(maxUses), mRewardExp(xp != 0), mDeprecated(false)
{
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> sell)
    : mBuyA(buyA), mBuyB(), mSell(sell), mUses(0), mMaxUses(12), mRewardExp(true), mDeprecated(false)
{
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, Item* sell)
    : mBuyA(buyA), mBuyB(), mSell(new ItemInstance(sell)), mUses(0), mMaxUses(12), mRewardExp(true), mDeprecated(false)
{
}

MerchantRecipe::MerchantRecipe(not_null_ptr<ItemInstance> buyA, Block* sell)
    : mBuyA(buyA), mBuyB(), mSell(new ItemInstance(sell)), mUses(0), mMaxUses(12), mRewardExp(true), mDeprecated(false)
{
}

void MerchantRecipe::_init(not_null_ptr<ItemInstance> buyA, not_null_ptr<ItemInstance> buyB, not_null_ptr<ItemInstance> sell)
{
    mBuyA = buyA; mBuyB = buyB; mSell = sell; mUses = 0; mMaxUses = 12; mRewardExp = true; mDeprecated = false;
}

void MerchantRecipe::load(CompoundTag* tag)
{
    if (tag == NULL) return;
    mUses = tag->getInt(L"uses");
    mMaxUses = tag->getInt(L"maxUses");
    mRewardExp = tag->getInt(L"rewardExp") != 0;
}

not_null_ptr<ItemInstance> MerchantRecipe::getBuyAItem() { return mBuyA; }
not_null_ptr<ItemInstance> MerchantRecipe::getBuyBItem() { return mBuyB; }
bool MerchantRecipe::hasSecondaryBuyItem() { return !mBuyB->isEmpty(); }
not_null_ptr<ItemInstance> MerchantRecipe::getSellItem() { return mSell; }
bool MerchantRecipe::isSame(MerchantRecipe* other)
{
    return other != NULL && ItemInstance::isSame(mBuyA, other->mBuyA) && ItemInstance::isSame(mSell, other->mSell);
}
bool MerchantRecipe::isSameSameButBetter(MerchantRecipe* other)
{
    return isSame(other) && other->mMaxUses > mMaxUses;
}
int MerchantRecipe::getUses() { return mUses; }
int MerchantRecipe::getMaxUses() { return mMaxUses; }
void MerchantRecipe::increaseUses() { ++mUses; }
void MerchantRecipe::increaseMaxUses(int amount) { mMaxUses += amount; }
bool MerchantRecipe::isDeprecated() { return mDeprecated || mUses >= mMaxUses; }
void MerchantRecipe::enforceDeprecated() { mDeprecated = true; }
bool MerchantRecipe::shouldRewardExp() { return mRewardExp; }
CompoundTag* MerchantRecipe::createTag()
{
    CompoundTag* tag = new CompoundTag();
    tag->putInt(L"uses", mUses);
    tag->putInt(L"maxUses", mMaxUses);
    tag->putInt(L"rewardExp", mRewardExp ? 1 : 0);
    return tag;
}
