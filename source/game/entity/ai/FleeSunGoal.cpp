#include "game/entity/ai/FleeSunGoal.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

FleeSunGoal::FleeSunGoal(PathfinderMob* mob, double speedModifier)
    : Goal(),
      mMob(mob),
      mWantedX(0.0),
      mWantedY(0.0),
      mWantedZ(0.0),
      mSpeedModifier(speedModifier),
      mLevel(((Entity*)mob)->mLevel)
{
    setRequiredControlFlags(1);
}

FleeSunGoal::~FleeSunGoal()
{
}

Vec3* FleeSunGoal::getHidePos()
{
    Random* random = mMob->getRandom();
    Entity* entity = (Entity*)mMob;
    BlockPos origin(entity->mX, entity->mY, entity->mZ);
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        BlockPos candidate = origin.offset(random->nextInt(20) - 10,
                                           random->nextInt(6) - 3,
                                           random->nextInt(20) - 10);
        if (!mLevel->canSeeSky(candidate) && mMob->getWalkTargetValue(candidate) < 0.0f)
            return Vec3::newTemp((double)candidate.x, (double)candidate.y, (double)candidate.z);

        candidate = origin.offset(random->nextInt(20) - 10,
                                  random->nextInt(6) - 3,
                                  random->nextInt(20) - 10);
        if (!mLevel->canSeeSky(candidate) && mMob->getWalkTargetValue(candidate) < 0.0f)
            return Vec3::newTemp((double)candidate.x, (double)candidate.y, (double)candidate.z);
    }

    return NULL;
}

bool FleeSunGoal::canUse()
{
    Entity* entity = (Entity*)mMob;
    if (!mLevel->isDay() || !mMob->isSunBurnTick() || entity->isInWaterOrRain())
        return false;

    BlockPos pos(entity->mX, entity->mY, entity->mZ);
    if (!mLevel->canSeeSky(pos))
        return false;

    not_null_ptr<ItemInstance> helmet = mMob->getItemSlot(EquipmentSlot::HEAD);
    if (!helmet->isEmpty())
        return false;

    Vec3* hidePos = getHidePos();
    if (hidePos == NULL)
        return false;

    mWantedX = hidePos->x;
    mWantedY = hidePos->y;
    mWantedZ = hidePos->z;
    return true;
}

bool FleeSunGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void FleeSunGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

void FleeSunGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring FleeSunGoal::getName()
{
    return std::wstring(L"FleeSun");
}
