#include "game/entity/ai/DoorInteractGoal.hpp"

#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

class GroundPathNavigation : public PathNavigation
{
public:
    bool canOpenDoors();
};

class DoorBlock
{
public:
    class DoorHalf;
    static AbstractProperty<DoorHalf*>* HALF;
    static DoorHalf* LOWER;
    static bool isDoor(Block* block);
};

DoorInteractGoal::DoorInteractGoal(Mob* mob)
    : Goal(),
      mMob(mob),
      mDoorPos(),
      mDoorBlock(NULL),
      mHasDoor(false),
      mDoorX(0.0f),
      mDoorZ(0.0f)
{
}

DoorInteractGoal::~DoorInteractGoal()
{
}

Block* DoorInteractGoal::getDoorBlock(const BlockPos& pos)
{
    const BlockState* state = mMob->mLevel->getBlockState(pos);
    Block* block = state->getBlock();
    if (DoorBlock::isDoor(block) && state->getValue(DoorBlock::HALF) == DoorBlock::LOWER)
        return block;
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
        if (path->getSize() < limit)
            limit = path->getSize();

        while (index < limit)
        {
            Node* node = path->get(index);
            BlockPos pos(node->x, node->y + 1, node->z);
            if (mMob->distanceToSqr((double)pos.x, mMob->mY, (double)pos.z) <= 2.25)
            {
                mDoorPos = pos;
                mDoorBlock = getDoorBlock(mDoorPos);
                if (mDoorBlock != NULL)
                    return true;
            }
            ++index;
            limit = path->getIndex() + 2;
            if (path->getSize() < limit)
                limit = path->getSize();
        }

        BlockPos pos(mMob->shared_from_this());
        mDoorPos = pos.above();
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
    mDoorX = (float)mDoorPos.x + 0.5f - (float)mMob->mX;
    mDoorZ = (float)mDoorPos.z + 0.5f - (float)mMob->mZ;
}

void DoorInteractGoal::tick()
{
    float x = (float)mDoorPos.x + 0.5f - (float)mMob->mX;
    float z = (float)mDoorPos.z + 0.5f - (float)mMob->mZ;
    if (mDoorX * x + mDoorZ * z < 0.0f)
        mHasDoor = true;
}

std::wstring DoorInteractGoal::getName()
{
    return std::wstring(L"DoorInteract");
}
