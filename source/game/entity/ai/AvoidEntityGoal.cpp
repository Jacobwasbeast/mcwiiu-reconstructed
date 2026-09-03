#include "game/entity/ai/AvoidEntityGoal.hpp"

#include "game/core/Path.hpp"
#include "game/entity/ai/PathNavigation.hpp"

AvoidEntityGoal::AvoidEntityGoal(PathfinderMob* mob, const std::type_info& avoidType, float maxDistance, double walkSpeedModifier, double sprintSpeedModifier)
    : Goal(),
      mMob(mob),
      mWalkSpeedModifier(walkSpeedModifier),
      mSprintSpeedModifier(sprintSpeedModifier),
      mMaxDistance(maxDistance),
      mPath(NULL),
      mNavigation(NULL),
      mAvoidType(&avoidType),
      mSelector(NULL),
      mToAvoid()
{
    mNavigation = NULL;
    mRequiredControlFlags = 1;
}

AvoidEntityGoal::AvoidEntityGoal(PathfinderMob* mob, const std::type_info& avoidType, const Predicate<boost::shared_ptr<Entity> >* selector, float maxDistance, double walkSpeedModifier, double sprintSpeedModifier)
    : Goal(),
      mMob(mob),
      mWalkSpeedModifier(walkSpeedModifier),
      mSprintSpeedModifier(sprintSpeedModifier),
      mMaxDistance(maxDistance),
      mPath(NULL),
      mNavigation(NULL),
      mAvoidType(&avoidType),
      mSelector(selector),
      mToAvoid()
{
    mNavigation = NULL;
    mRequiredControlFlags = 1;
}

AvoidEntityGoal::~AvoidEntityGoal()
{
    if (mPath != NULL)
        delete mPath;
}

void AvoidEntityGoal::_init(PathfinderMob* mob, const Predicate<boost::shared_ptr<Entity> >* selector, float maxDistance, double walkSpeedModifier, double sprintSpeedModifier)
{
    mMob = mob;
    mSelector = selector;
    mMaxDistance = maxDistance;
    mWalkSpeedModifier = walkSpeedModifier;
    mSprintSpeedModifier = sprintSpeedModifier;
    mPath = NULL;
    mToAvoid.reset();
    mRequiredControlFlags = 1;
}

bool AvoidEntityGoal::canUse()
{
    return mNavigation != NULL && mPath != NULL;
}

bool AvoidEntityGoal::canContinueToUse()
{
    return mToAvoid.get() != NULL && mNavigation != NULL && !mNavigation->isDone();
}

void AvoidEntityGoal::start()
{
    if (mNavigation != NULL && mPath != NULL)
    {
        mNavigation->moveTo(mPath, mWalkSpeedModifier);
        mPath = NULL;
    }
}

void AvoidEntityGoal::stop()
{
    mToAvoid.reset();
    if (mPath != NULL)
    {
        delete mPath;
        mPath = NULL;
    }
}

void AvoidEntityGoal::tick()
{
    if (mNavigation != NULL)
        mNavigation->setSpeedModifier(mWalkSpeedModifier);
}

std::wstring AvoidEntityGoal::getName()
{
    return std::wstring(L"AvoidEntity");
}
