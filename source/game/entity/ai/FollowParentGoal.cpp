#include "game/entity/ai/FollowParentGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

class Animal : public Entity
{
public:
    virtual int getAge();
    virtual PathNavigation* getNavigation();
};

FollowParentGoal::FollowParentGoal(Animal* animal, double speedModifier)
    : mAnimal(animal), mParentAnimal(), mSpeedModifier(speedModifier), mTimeToRecalcPath(0)
{
}

FollowParentGoal::~FollowParentGoal()
{
}

bool FollowParentGoal::canUse()
{
    if (mAnimal->getAge() >= 0)
        return false;

    if (!mParentAnimal)
    {
        AABB* searchBox = ((Entity*)mAnimal)->mBoundingBox->grow(8.0, 4.0, 8.0);
        std::vector<boost::shared_ptr<Entity> >* animals =
            ((Entity*)mAnimal)->mLevel->getEntitiesOfClass(typeid(*mAnimal), searchBox);
        boost::shared_ptr<Animal> closest;
        double closestDistance = 1.7976931348623157e+308;

        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = animals->begin(); it != animals->end(); ++it)
        {
            boost::shared_ptr<Animal> candidate = boost::dynamic_pointer_cast<Animal>(*it);
            if (!candidate || candidate->getAge() < 0)
                continue;

            double distance = ((Entity*)mAnimal)->distanceToSqr(*it);
            if (distance <= closestDistance)
            {
                closestDistance = distance;
                closest = candidate;
            }
        }

        delete animals;
        if (!closest || closestDistance < 9.0)
            return false;

        mParentAnimal = closest;
    }

    return true;
}

bool FollowParentGoal::canContinueToUse()
{
    if (mAnimal->getAge() >= 0 || !mParentAnimal)
        return false;

    double distance = ((Entity*)mAnimal)->distanceToSqr(mParentAnimal);
    return distance >= 9.0 && distance <= 256.0;
}

void FollowParentGoal::start()
{
    mTimeToRecalcPath = 0;
}

void FollowParentGoal::stop()
{
    mParentAnimal.reset();
}

void FollowParentGoal::tick()
{
    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = 10;
        mAnimal->getNavigation()->moveTo(boost::static_pointer_cast<Entity>(mParentAnimal),
                                         mSpeedModifier);
    }
}

std::wstring FollowParentGoal::getName()
{
    return std::wstring(L"FollowParentGoal");
}
