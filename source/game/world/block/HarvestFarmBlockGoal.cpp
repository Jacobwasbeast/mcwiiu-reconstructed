#include "game/world/block/HarvestFarmBlockGoal.hpp"

#include "game/entity/ai/LookControl.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Villager.hpp"

class CropBlock
{
public:
    static bool isMaxAge(const BlockState* state);
};

static bool isCropBlock(Block* block)
{
    return block == Blocks::wheat || block == Blocks::carrots ||
           block == Blocks::potatoes || block == Blocks::beetroots;
}

HarvestFarmBlockGoal::HarvestFarmBlockGoal(Villager* villager,
                                           double speedModifier)
    : MoveToBlockGoal(villager, speedModifier, 16, 1, 1.0),
      mVillager(villager),
      mHasFarmSeeds(false),
      mWantsMoreFood(false),
      mTargetType(0)
{
}

HarvestFarmBlockGoal::~HarvestFarmBlockGoal()
{
}

unsigned int HarvestFarmBlockGoal::fjGetNextFarmTarget()
{
    mTargetType = -1;
    mHasFarmSeeds = mVillager->hasFarmSeeds();
    mWantsMoreFood = mVillager->wantsMoreFood();
    if (!findNearestBlock())
    {
        return 0;
    }
    return isValidTarget(mVillager->getLevel(), mBlockPos);
}

bool HarvestFarmBlockGoal::canUse()
{
    if (!mVillager->getLevel()->getGameRules()->getBoolean(1))
    {
        return false;
    }
    if (mNextStartTick > 0)
    {
        --mNextStartTick;
        return false;
    }
    mNextStartTick = mVillager->getRandom()->nextInt(200);
    return fjGetNextFarmTarget() != 0;
}

bool HarvestFarmBlockGoal::canContinueToUse()
{
    return mTargetType >= 0 && MoveToBlockGoal::canContinueToUse();
}

int HarvestFarmBlockGoal::findInventorySlotForFarmSeeds()
{
    Inventory* inventory = mVillager->getInventory();
    for (int slot = 0; slot < (int)inventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = inventory->getItem(slot);
        if (item->isEmpty())
        {
            continue;
        }
        Item* itemType = item->getItem();
        if (itemType == Items::wheatSeeds || itemType == Items::carrot ||
            itemType == Items::potato || itemType == Items::beetrootSeeds)
        {
            return slot;
        }
    }
    return -1;
}

void HarvestFarmBlockGoal::tick()
{
    MoveToBlockGoal::tick();
    mVillager->getLookControl()->setLookAt(
        (double)mBlockPos.x + 0.5, (double)mBlockPos.y + 1.0,
        (double)mBlockPos.z + 0.5, 10.0f, mVillager->getMaxHeadXRot());

    if (!isReachedTarget())
    {
        return;
    }

    Level* level = mVillager->getLevel();
    BlockPos above = mBlockPos.above();
    const BlockState* state = level->getBlockState(above);
    Block* block = state->getBlock();

    if (block == Blocks::AIR)
    {
        if (mTargetType == 1)
        {
            int slot = findInventorySlotForFarmSeeds();
            if (slot < 0)
            {
                mTargetType = -1;
                mNextStartTick = 0;
                return;
            }
            Inventory* inventory = mVillager->getInventory();
            not_null_ptr<ItemInstance> item = inventory->getItem(slot);
            Item* itemType = item->getItem();
            if (itemType == Items::wheatSeeds)
            {
                level->setBlock(above, Blocks::wheat->defaultBlockState(), 3,
                                false);
            }
            else if (itemType == Items::carrot)
            {
                level->setBlock(above, Blocks::carrots->defaultBlockState(), 3,
                                false);
            }
            else if (itemType == Items::potato)
            {
                level->setBlock(above, Blocks::potatoes->defaultBlockState(), 3,
                                false);
            }
            else if (itemType == Items::beetrootSeeds)
            {
                level->setBlock(above, Blocks::beetroots->defaultBlockState(), 3,
                                false);
            }
            item->shrink(1);
            if (item->isEmpty())
            {
                inventory->setItem(slot, ItemInstance::EMPTY);
            }
            if (fjGetNextFarmTarget() != 0)
            {
                start();
                return;
            }
        }
    }
    else if (mTargetType == 0)
    {
        if (isCropBlock(block) && CropBlock::isMaxAge(state))
        {
            level->destroyBlock(above, true);
            mTargetType = -1;
            mNextStartTick = 10;
            return;
        }
    }

    mTargetType = -1;
    mNextStartTick = 10;
}

bool HarvestFarmBlockGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    const BlockState* farmlandState = level->getBlockState(pos);
    if (farmlandState->getBlock() != Blocks::farmland)
    {
        return false;
    }

    BlockPos above = pos.above();
    const BlockState* aboveState = level->getBlockState(above);
    Block* aboveBlock = aboveState->getBlock();
    if (mHasFarmSeeds && aboveBlock == Blocks::AIR)
    {
        mTargetType = 1;
        return true;
    }
    if (mWantsMoreFood && isCropBlock(aboveBlock) &&
        (mTargetType == 0 || mTargetType == -1) &&
        CropBlock::isMaxAge(aboveState))
    {
        mTargetType = 0;
        return true;
    }
    return false;
}

std::wstring HarvestFarmBlockGoal::getName()
{
    return std::wstring(L"HarvestFarmBlock");
}
