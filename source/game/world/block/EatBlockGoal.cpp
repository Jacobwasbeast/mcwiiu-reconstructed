#include "game/world/block/EatBlockGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Boxed.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"

class Property;

extern bool tallGrassPredicate(Boxed* value);
extern Property* TallGrassBlock_VARIANT;

static Predicate<const BlockState*>* sTallGrassPredicate;

EatBlockGoal::EatBlockGoal(Mob* mob)
    : Goal(), mMob(mob), mLevel(mob->mLevel), mEatAnimationTick(0)
{
    setRequiredControlFlags(7);
}

EatBlockGoal::~EatBlockGoal()
{
}

void EatBlockGoal::staticCtor()
{
    sTallGrassPredicate = BlockStatePredicate::forBlock(Blocks::tallgrass);
    static_cast<BlockStatePredicate*>(sTallGrassPredicate)->where(
        TallGrassBlock_VARIANT,
        new Predicates<Boxed*>::FunctionPredicate(tallGrassPredicate));
}

bool EatBlockGoal::canUse()
{
    int interval = mMob->isBaby() ? 50 : 1000;
    if (mMob->getRandom()->nextInt(interval) != 0)
        return false;

    BlockPos pos(mMob->mX, mMob->mY, mMob->mZ);
    if (sTallGrassPredicate->apply(mLevel->getBlockState(pos)))
        return true;

    return mLevel->getBlockState(pos.below())->getBlock() == Blocks::grass;
}

void EatBlockGoal::start()
{
    mEatAnimationTick = 40;
    mLevel->broadcastEntityEvent(mMob->shared_from_this(), 10, 0);
    mMob->getNavigation()->stop();
}

void EatBlockGoal::stop()
{
    mEatAnimationTick = 0;
}

bool EatBlockGoal::canContinueToUse()
{
    return mEatAnimationTick != 0;
}

int EatBlockGoal::getEatAnimationTick()
{
    return mEatAnimationTick;
}

void EatBlockGoal::tick()
{
    mEatAnimationTick = mEatAnimationTick - 1;
    if (mEatAnimationTick < 0)
        mEatAnimationTick = 0;

    if (mEatAnimationTick != 4)
        return;

    BlockPos pos(mMob->mX, mMob->mY, mMob->mZ);
    if (sTallGrassPredicate->apply(mLevel->getBlockState(pos)))
    {
        if (mLevel->getGameRules()->getBoolean(1))
            mLevel->levelEvent(2001, pos, Block::getId(Blocks::grass));
        mMob->getNavigation()->stop();
        return;
    }

    BlockPos below = pos.below();
    if (mLevel->getBlockState(below)->getBlock() == Blocks::grass &&
        mLevel->getGameRules()->getBoolean(1))
    {
        mLevel->levelEvent(2001, below, Block::getId(Blocks::grass));
        mLevel->setBlock(below, Blocks::dirt->defaultBlockState(), 2, false);
    }
    mMob->getNavigation()->stop();
}

void EatBlockGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring EatBlockGoal::getName()
{
    return std::wstring(L"EatBlock");
}
