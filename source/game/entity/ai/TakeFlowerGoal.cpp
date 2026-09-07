#include "game/entity/ai/TakeFlowerGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

TakeFlowerGoal::TakeFlowerGoal(Villager* villager)
    : mVillager(villager), mGolem(), mTicks(0), mWantsToTakeFlower(false)
{
    setRequiredControlFlags(3);
}

TakeFlowerGoal::~TakeFlowerGoal()
{
}

bool TakeFlowerGoal::canUse()
{
    if (mVillager->getAge() >= 0 || !mVillager->mLevel->isDay())
        return false;

    AABB* area = mVillager->mBoundingBox->grow(6.0, 2.0, 6.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mVillager->mLevel->getEntitiesOfClass(typeid(VillagerGolem), area);

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<VillagerGolem> golem =
            boost::dynamic_pointer_cast<VillagerGolem>(*it);
        if (golem && golem->getOfferFlowerTick() > 0)
        {
            mGolem = golem;
            delete entities;
            return true;
        }
    }

    delete entities;
    return false;
}

bool TakeFlowerGoal::canContinueToUse()
{
    return mGolem && mGolem->getOfferFlowerTick() > 0;
}

void TakeFlowerGoal::start()
{
    mTicks = mVillager->getRandom()->nextInt(320);
    mWantsToTakeFlower = false;
    mVillager->getNavigation()->stop();
}

void TakeFlowerGoal::stop()
{
    mGolem.reset();
    mVillager->getNavigation()->stop();
}

void TakeFlowerGoal::tick()
{
    mVillager->getLookControl()->setLookAt(mGolem, 30.0f, 30.0f);

    if (mGolem->getOfferFlowerTick() == mTicks)
    {
        if (!mWantsToTakeFlower)
        {
            mVillager->getNavigation()->moveTo(mGolem, 0.5);
            mWantsToTakeFlower = true;
        }
    }
    else if (!mWantsToTakeFlower)
    {
        return;
    }

    if (mVillager->distanceToSqr(mGolem) < 4.0)
    {
        mGolem->offerFlower(false);
        mVillager->getNavigation()->stop();
    }
}

std::wstring TakeFlowerGoal::getName()
{
    return L"TakeFlower";
}
