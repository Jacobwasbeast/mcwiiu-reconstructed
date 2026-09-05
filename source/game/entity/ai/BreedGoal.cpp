#include "game/entity/ai/BreedGoal.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <typeinfo>
#include <vector>
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

BreedGoal::BreedGoal(Animal* animal, double speedModifier)
    : mAnimal(animal), mPartner(), mLevel(animal != 0 ? animal->mLevel : 0),
      mPartnerType(animal != 0 ? &typeid(*animal) : &typeid(Animal)), mTicks(0),
      mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(3);
}

BreedGoal::BreedGoal(Animal* animal, double speedModifier, const std::type_info& partnerType)
    : mAnimal(animal), mPartner(), mLevel(animal != 0 ? animal->mLevel : 0),
      mPartnerType(&partnerType), mTicks(0), mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(3);
}

BreedGoal::~BreedGoal()
{
}

void BreedGoal::getFreePartner()
{
    mPartner.reset();
    if (mAnimal == 0 || mLevel == 0) {
        return;
    }

    AABB area = mAnimal->mBoundingBox->grow(8.0);
    std::vector<boost::shared_ptr<Entity> > entities =
        mLevel->getEntitiesOfClass(*mPartnerType, &area);
    double nearest = 1.7976931348623157e+308;
    std::vector<boost::shared_ptr<Entity> >::const_iterator it;
    for (it = entities.begin(); it != entities.end(); ++it) {
        boost::shared_ptr<Animal> animal = boost::dynamic_pointer_cast<Animal>(*it);
        if (animal && mAnimal->canMate(animal)) {
            double distance = mAnimal->distanceToSqr(*it);
            if (distance < nearest) {
                nearest = distance;
                mPartner = animal;
            }
        }
    }
}

bool BreedGoal::canUse()
{
    if (mAnimal == 0 || !mAnimal->isInLove()) {
        return false;
    }
    getFreePartner();
    return !mPartner.empty();
}

bool BreedGoal::canContinueToUse()
{
    return !mPartner.empty() && mPartner->isAlive() && mPartner->isInLove() && mTicks < 60;
}

void BreedGoal::stop()
{
    mPartner.reset();
    mTicks = 0;
}

void BreedGoal::tick()
{
    ++mTicks;
    if (mAnimal == 0 || mPartner.empty()) {
        return;
    }
    mAnimal->getLookControl()->setLookAt(mPartner, 10.0F, 10.0F);
    if (mTicks >= 60 && mAnimal->distanceToSqr(mPartner) < 9.0) {
        breed();
    }
}

void BreedGoal::breed()
{
    if (mAnimal == 0 || mPartner.empty() || mLevel == 0) {
        return;
    }
    if (!mAnimal->canMate(mPartner) || !mPartner->isInLove()) {
        return;
    }
    mAnimal->resetLove();
    mPartner->resetLove();
    mTicks = 0;
}

void BreedGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring BreedGoal::getName()
{
    return L"Breed";
}
