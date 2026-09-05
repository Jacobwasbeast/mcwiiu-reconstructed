#ifndef GAME_ENTITY_AI_FOLLOWPARENTGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWPARENTGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Animal;

class FollowParentGoal : public Goal
{
public:
    FollowParentGoal(Animal* animal, double speedModifier);
    virtual ~FollowParentGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    Animal* mAnimal;
    boost::shared_ptr<Animal> mParentAnimal;
    double mSpeedModifier;
    int mTimeToRecalcPath;
};

#endif
