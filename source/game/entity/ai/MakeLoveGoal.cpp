#include "game/entity/ai/MakeLoveGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/block/BlockPos.hpp"

MakeLoveGoal::MakeLoveGoal(Villager* villager)
    : mVillager(villager), mPartner(), mLevel(villager->mLevel),
      mRunningTicks(0), mVillage()
{
    setRequiredControlFlags(3);
}

MakeLoveGoal::~MakeLoveGoal()
{
    mPartner.reset();
    mVillage.reset();
    mVillager = 0;
    mLevel = 0;
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
    if (!mVillage || !mVillage->isBreedTimerOk())
        return false;

    return mVillage->getPopulationSize() < (int)(mVillage->getDoorCount() * 0.35);
}

bool MakeLoveGoal::canUse()
{
    if (mVillager->getAge() != 0)
        return false;

    if (mVillager->getRandom()->nextInt(500) != 0)
        return false;

    boost::shared_ptr<Villages> villages = mLevel->getVillages();
    if (!villages)
        return false;

    boost::shared_ptr<Entity> self = mVillager->shared_from_this();
    BlockPos position(self);
    mVillage = villages->getClosestVillage(position, 0);
    if (!mVillage || !villageNeedsMoreVillagers()
        || !mVillager->isWillingToBreed(true))
        return false;

    AABB* area = mVillager->mBoundingBox->grow(8.0, 3.0, 8.0);
    boost::shared_ptr<Entity> entity =
        mLevel->getClosestEntityOfClass(typeid(Villager), area, self);
    if (!entity)
        return false;

    mPartner = boost::dynamic_pointer_cast<Villager>(entity);
    return mPartner && mPartner->getAge() == 0 && mPartner->isWillingToBreed(true);
}

void MakeLoveGoal::start()
{
    mRunningTicks = 300;
    mVillager->setInLove(true);
}

void MakeLoveGoal::stop()
{
    mVillage.reset();
    mPartner.reset();
    mVillager->setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
    return mPartner && mRunningTicks > -1 && villageNeedsMoreVillagers()
        && mVillager->getAge() == 0 && mVillager->isWillingToBreed(false);
}

void MakeLoveGoal::breed()
{
    mPartner->setAge(6000);
    mVillager->setAge(6000);

    if (!mLevel->canCreateMore(EntityType::VILLAGER, Level::eSpawnType_Breed))
        return;

    boost::shared_ptr<AgableMob> child = mVillager->getBreedOffspring(mPartner);
    boost::shared_ptr<Villager> villager =
        boost::dynamic_pointer_cast<Villager>(child);

    mPartner->setWillingToBreed(false);
    mVillager->setWillingToBreed(false);
    villager->setAge(-24000);
    villager->setPos(mVillager->mX, mVillager->mY, mVillager->mZ);
    mLevel->addEntity(villager);
    mLevel->broadcastEntityEvent(villager, EntityEvent::LOVE_HEARTS, 0);
}

void MakeLoveGoal::tick()
{
    --mRunningTicks;

    mVillager->getLookControl()->setLookAt(mPartner, 10.0F, 30.0F);
    if (mVillager->distanceToSqr(mPartner) <= 2.25) {
        if (mRunningTicks == 0 && mPartner->isInLove())
            breed();
    } else {
        mVillager->getNavigation()->moveTo(mPartner, 0.25);
    }

    if (mVillager->getRandom()->nextInt(35) == 0)
        mLevel->broadcastEntityEvent(mVillager->shared_from_this(), EntityEvent::LOVE_HEARTS, 0);
}

void MakeLoveGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring MakeLoveGoal::getName()
{
    return std::wstring(L"MakeLove");
}
