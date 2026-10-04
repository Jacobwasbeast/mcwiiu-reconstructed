#include "game/core/Sensing.hpp"

#include "game/entity/mob/Mob.hpp"

Sensing::Sensing(Mob* mob)
{
    mMob = mob;
}

void Sensing::tick()
{
    mSeen.clear();
    mUnseen.clear();
}

bool Sensing::canSee(boost::shared_ptr<Entity> entity)
{
    std::vector<boost::weak_ptr<Entity> >::iterator seen = mSeen.begin();
    for (; seen != mSeen.end(); ++seen)
    {
        if (seen->lock() == entity)
            return true;
    }

    std::vector<boost::weak_ptr<Entity> >::iterator unseen = mUnseen.begin();
    for (; unseen != mUnseen.end(); ++unseen)
    {
        if (unseen->lock() == entity)
            return false;
    }

    bool visible = mMob->canSee(entity);
    if (visible)
    {
        mSeen.push_back(boost::weak_ptr<Entity>(entity));
    }
    else
    {
        mUnseen.push_back(boost::weak_ptr<Entity>(entity));
    }
    return visible;
}
