#ifndef GAME_ENTITY_AI_MOVETOWARDSRESTRICTIONGOAL_HPP
#define GAME_ENTITY_AI_MOVETOWARDSRESTRICTIONGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class PathfinderMob;

class MoveTowardsRestrictionGoal : public Goal
{
public:
    MoveTowardsRestrictionGoal(PathfinderMob* mob, double speedModifier);
    virtual ~MoveTowardsRestrictionGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual std::wstring getName();

protected:
    PathfinderMob* mMob;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
};

#endif
