#include "game/entity/ai/DoorInteractGoal.hpp"

#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"

DoorInteractGoal::DoorInteractGoal(Mob* mob)
    : Goal(),
      mDoorPos(),
      mDoorBlock(NULL),
      mHasDoor(false),
      mDoorX(0.0f),
      mDoorZ(0.0f)
{
    mDoorPos = BlockPos::ZERO;
    mMob = mob;
}

Block* DoorInteractGoal::getDoorBlock(const BlockPos& pos)
{
    const BlockState* state = mMob->mLevel->getBlockState(pos);
    Block* block = state->getBlock();
    if (DoorBlock::isDoor(block) && state->getMaterial() == Material::WOOD)
        return state->getBlock();
    return NULL;
}

bool DoorInteractGoal::canUse()
{
    if (!mMob->mHorizontalCollision)
        return false;

    PathNavigation* navigation = mMob->getNavigation();
    Path* path = navigation->getPath();
    GroundPathNavigation* groundNavigation = static_cast<GroundPathNavigation*>(navigation);
    if (path != NULL && !path->isDone() && groundNavigation->canOpenDoors())
    {
        int index = 0;
        int limit = path->getIndex() + 2;
        int size = path->getSize();
        if (size < limit)
            limit = size;

        while (index < limit)
        {
            Node* node = path->get(index);
            mDoorPos = BlockPos(node->x, node->y + 1, node->z);
            if (mMob->distanceToSqr((double)mDoorPos.x, mMob->mY, (double)mDoorPos.z) <= 2.25)
            {
                mDoorBlock = getDoorBlock(mDoorPos);
                if (mDoorBlock != NULL)
                    return true;
            }
            ++index;
            limit = path->getIndex() + 2;
            size = path->getSize();
            if (size < limit)
                limit = size;
        }

        mDoorPos = BlockPos(mMob->shared_from_this()).above();
        mDoorBlock = getDoorBlock(mDoorPos);
        return mDoorBlock != NULL;
    }

    return false;
}

bool DoorInteractGoal::canContinueToUse()
{
    return !mHasDoor;
}

void DoorInteractGoal::start()
{
    mHasDoor = false;
    mDoorX = (float)((double)((float)mDoorPos.x + 0.5f) - mMob->mX);
    mDoorZ = (float)((double)((float)mDoorPos.z + 0.5f) - mMob->mZ);
}

void DoorInteractGoal::tick()
{
    float x = (float)((double)((float)mDoorPos.x + 0.5f) - mMob->mX);
    float z = (float)((double)((float)mDoorPos.z + 0.5f) - mMob->mZ);
    if (mDoorX * x + mDoorZ * z < 0.0f)
        mHasDoor = true;
}

std::wstring DoorInteractGoal::getName()
{
    return std::wstring(L"DoorInteract");
}
