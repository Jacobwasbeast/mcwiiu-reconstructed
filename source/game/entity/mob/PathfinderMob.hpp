#ifndef GAME_ENTITY_MOB_PATHFINDERMOB_HPP
#define GAME_ENTITY_MOB_PATHFINDERMOB_HPP

#include "game/entity/mob/Mob.hpp"
#include "game/world/block/BlockPos.hpp"

class Level;
class Path;

class PathfinderMob : public Mob
{
public:
    PathfinderMob(Level* level);
    virtual ~PathfinderMob();

    virtual double getWalkTargetValue(const BlockPos& pos);
    virtual bool checkSpawnRules();
    virtual bool isPathFinding();
    virtual bool isWithinRestriction();
    virtual bool isWithinRestriction(const BlockPos& pos);
    virtual void restrictTo(const BlockPos& pos, int radius);
    virtual BlockPos getRestrictCenter();
    virtual double getRestrictRadius();
    virtual void clearRestriction();
    virtual bool hasRestriction();
    virtual void tickLeash();
    virtual void onLeashDistance(float distance);
    virtual double followLeashSpeed();
    virtual bool couldWander();

private:
    Path* mPath;
    int mPathTick;
    void* mPathOwner;
    bool mPathValid;
    int mPathDistance;
    BlockPos mRestrictionCenter;
    float mRestrictionRadius;
    float mWalkTargetValue;
};

#endif
