#ifndef GAME_ENTITY_AI_PATHNAVIGATION_HPP
#define GAME_ENTITY_AI_PATHNAVIGATION_HPP

#include <boost/shared_ptr.hpp>
#include "game/world/block/BlockPos.hpp"
#include "game/util/Vec3.hpp"

class Entity;
class Level;
class Mob;
class Node;
class Path;
class PathFinder;

class PathNavigation
{
public:
    Path* mPath;
    Mob* mMob;
    Level* mLevel;
    int mTick;
    double mSpeedModifier;
    BlockPos* mTargetPos;
    int mMaxDistance;
    int mLastStuckCheck;
    Vec3* mLastStuckCheckPos;
    Vec3* mLastTargetPos;
    int mStuckTime;
    int mLastStuckCheckTime;
    int mLastStuckCheckTimeHigh;
    int mTimeoutCachedNode;
    double mTimeoutLimit;
    bool mIsRiverFollowing;
    float mMaxDistanceToWaypoint;
    bool mHasDelayedRecomputation;
    int mRecomputePathTime;
    int mLastRecomputeTime;
    Path* mTargetPath;
    PathFinder* mPathFinder;

    PathNavigation(Mob* mob, Level* level);
    virtual ~PathNavigation();
    virtual bool canUpdatePath() = 0;
    virtual Path* createPath(const BlockPos& pos);
    virtual Path* createPath(boost::shared_ptr<Entity> entity);
    virtual Path* createDirectPath(const BlockPos& pos);
    virtual bool moveTo(double x, double y, double z, double speed);
    virtual bool moveTo(boost::shared_ptr<Entity> entity, double speed);
    virtual bool moveTo(Path* path, double speed);
    virtual void tick();
    virtual bool shouldAvoidSun() const;
    virtual void updatePath();
    virtual Vec3* getTempMobPos() = 0;
    virtual bool canMoveDirectly(Vec3* a, Vec3* b, int x, int y, int z) = 0;
    virtual void trimPath();
    virtual bool isStableDestination(const BlockPos& pos);

    bool isDone();
    void stop();
    PathFinder* getPathFinder();
    void setPathFinder(PathFinder* pathFinder);
    void setSpeedModifier(double speed);
    double getMaxDist();
    bool hasDelayedRecomputation();
    void recomputePath();
    Path* createPath(double x, double y, double z);
    Path* getPath();
    void debugPathfinding();
    void doStuckDetection(Vec3* pos);
    bool isInLiquid();
    void setLevel(Level* level);
    bool isRiverFollowing();
    void setIsRiverFollowing(bool value);
};

#endif
