#include "game/entity/ai/BreedGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <typeinfo>
#include <vector>

#include "game/core/Path.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

BreedGoal::BreedGoal(Animal* animal, double speedModifier)
    : mAnimal(animal), mPartner(), mLevel(animal->mLevel),
      mPartnerType(&typeid(*animal)), mTicks(0), mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(3);
}

BreedGoal::BreedGoal(Animal* animal, double speedModifier, const std::type_info& partnerType)
    : mAnimal(animal), mPartner(), mLevel(animal->mLevel),
      mPartnerType(&partnerType), mTicks(0), mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(3);
}

BreedGoal::~BreedGoal()
{
}

boost::shared_ptr<Animal> BreedGoal::getFreePartner()
{
    AABB* area = mAnimal->mBoundingBox->grow(8.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntitiesOfClass(*mPartnerType, area);
    boost::shared_ptr<Animal> partner;
    double nearest = 1.7976931348623157e+308;
    std::vector<boost::shared_ptr<Entity> >::const_iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Animal> candidate = boost::dynamic_pointer_cast<Animal>(*it);
        if (mAnimal->canMate(candidate) && mAnimal->distanceToSqr(candidate) < nearest)
        {
            partner = candidate;
            nearest = mAnimal->distanceToSqr(candidate);
        }
    }
    delete entities;
    return partner;
}

bool BreedGoal::canUse()
{
    if (!mAnimal->isInLove())
        return false;

    mPartner = getFreePartner();
    return !mPartner.expired();
}

bool BreedGoal::canContinueToUse()
{
    boost::shared_ptr<Animal> partner = mPartner.lock();
    if (!partner)
        return false;

    if (!partner->isAlive())
        return false;

    if (!partner->isInLove())
        return false;

    return mTicks < 60;
}

void BreedGoal::stop()
{
    mPartner.reset();
    mTicks = 0;
}

void BreedGoal::tick()
{
    ++mTicks;
    boost::shared_ptr<Animal> partner = mPartner.lock();
    mAnimal->getLookControl()->setLookAt(partner, 10.0f,
                                         static_cast<float>(mAnimal->getMaxHeadXRot()));
    mAnimal->getNavigation()->moveTo(partner, mSpeedModifier);
    if (mTicks > 59)
    {
        BlockPos pos(partner);
        Path* path = mAnimal->getNavigation()->getPath();
        if (path != NULL && path->endsIn(Vec3::newTemp(&pos)) && path->IsWithinStepsToTheEnd(1) &&
            mAnimal->getSensing()->canSee(partner) && mAnimal->distanceToSqr(partner) < 9.0)
            breed();
    }
}

void BreedGoal::breed()
{
    boost::shared_ptr<Animal> partner = mPartner.lock();
    mAnimal->setDespawnProtected();
    partner->setDespawnProtected();
    if (!mLevel->canCreateMore(mAnimal->GetType(), 1))
    {
        mAnimal->resetLove();
        partner->resetLove();
        return;
    }
}

void BreedGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring BreedGoal::getName()
{
    return std::wstring(L"Breed");
}
