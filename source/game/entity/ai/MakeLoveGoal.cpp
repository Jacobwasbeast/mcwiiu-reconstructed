#include "game/entity/ai/MakeLoveGoal.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/block/BlockPos.hpp"

MakeLoveGoal::MakeLoveGoal(Villager* villager)
    : mVillager(villager), mVillage(), mLevel(villager->mLevel),
      mRunningTicks(0), mPartner()
{
    setRequiredControlFlags(3);
}

MakeLoveGoal::~MakeLoveGoal()
{
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
    if (!mVillage || !mVillage->isBreedTimerOk())
        return false;

    return mVillage->getPopulationSize() < mVillage->getDoorCount() * 0.35;
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
    mVillage = villages->getClosestVillage(position, 32);
    if (!mVillage || !villageNeedsMoreVillagers()
        || !mVillager->isWillingToBreed(true))
        return false;

    AABB* area = mVillager->mBoundingBox->grow(8.0, 4.0, 8.0);
    boost::shared_ptr<Entity> entity =
        mLevel->getClosestEntityOfClass(typeid(Villager), area, self);
    delete area;
    if (!entity)
        return false;

    mPartner = boost::dynamic_pointer_cast<Villager>(entity);
    return mPartner && mPartner->isWillingToBreed(true);
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
    mRunningTicks = 0;
    mVillager->setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
    return mPartner && mRunningTicks > -1 && villageNeedsMoreVillagers()
        && mVillager->getAge() == 0 && mVillager->isWillingToBreed(false);
}

void MakeLoveGoal::breed()
{
    if (!mVillage || !mPartner)
        return;

    mVillage->resetNoBreedTimer();
    mVillager->setWillingToBreed(false);
    mPartner->setWillingToBreed(false);
    mVillager->setAge(6000);
    mPartner->setAge(6000);

    if (!mLevel->canCreateMore(0x000000000D600001ULL, 1))
        return;

    boost::shared_ptr<AgableMob> parent =
        boost::dynamic_pointer_cast<AgableMob>(mVillager->shared_from_this());
    boost::shared_ptr<AgableMob> child = mPartner->getBreedOffspring(parent);
    boost::shared_ptr<Villager> villager =
        boost::dynamic_pointer_cast<Villager>(child);
    if (!villager)
        return;

    villager->setAge(-24000);
    villager->setPos(mVillager->mX, mVillager->mY, mVillager->mZ);
    mLevel->addEntity(villager);
    mLevel->broadcastEntityEvent(villager, 12, 0);
}

void MakeLoveGoal::tick()
{
    --mRunningTicks;
    if (!mPartner)
        return;

    mVillager->getLookControl()->setLookAt(mPartner, 10.0F, 10.0F);
    if (mVillager->distanceToSqr(mPartner) <= 2.25) {
        if (mRunningTicks == 0 && mPartner->isInLove())
            breed();
    } else if (mVillager->getRandom()->nextInt(35) == 0) {
        mVillager->getLookControl()->setLookAt(mPartner, 10.0F, 10.0F);
    }
}

void MakeLoveGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring MakeLoveGoal::getName()
{
    return std::wstring(L"MakeLove");
}
