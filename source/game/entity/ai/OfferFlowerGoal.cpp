#include "game/entity/ai/OfferFlowerGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <typeinfo>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

OfferFlowerGoal::OfferFlowerGoal(VillagerGolem* golem)
    : mGolem(golem), mTarget(), mTicks(0)
{
    setRequiredControlFlags(3);
}

OfferFlowerGoal::~OfferFlowerGoal()
{
    mTarget.reset();
    mGolem = 0;
}

bool OfferFlowerGoal::canUse()
{
    if (!mGolem->mLevel->isDay())
        return false;
    if (mGolem->getRandom()->nextInt(8000) != 0)
        return false;

    AABB* area = mGolem->mBoundingBox->grow(6.0, 2.0, 6.0);
    boost::shared_ptr<Entity> closest =
        mGolem->mLevel->getClosestEntityOfClass(typeid(Villager), area, mGolem->shared_from_this());
    mTarget = boost::dynamic_pointer_cast<Villager>(closest);
    return !!mTarget.lock();
}

bool OfferFlowerGoal::canContinueToUse()
{
    return mTicks > 0 && mTarget.lock();
}

void OfferFlowerGoal::start()
{
    mTicks = 400;
    mGolem->offerFlower(true);
}

void OfferFlowerGoal::stop()
{
    mGolem->offerFlower(false);
    mTarget.reset();
}

void OfferFlowerGoal::tick()
{
    mGolem->getLookControl()->setLookAt(mTarget.lock(), 30.0f, 30.0f);
    --mTicks;
}

std::wstring OfferFlowerGoal::getName()
{
    return L"OfferFlower";
}
