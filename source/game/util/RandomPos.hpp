#ifndef GAME_UTIL_RANDOMPOS_HPP
#define GAME_UTIL_RANDOMPOS_HPP

#include <boost/shared_ptr.hpp>

class BlockPos;
class PathfinderMob;
class Random;
class Vec3;

namespace RandomPos
{
    Vec3* getPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, int wanderingQuadrant);
    Vec3* getLandPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, int wanderingQuadrant);
    Vec3* getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target);
    Vec3* getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target, double maxAngleDelta);
    Vec3* getPosAvoid(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* avoid);

    BlockPos* getRandomDelta(Random* random, int xz, int y, const Vec3* direction, double maxAngleDelta);
    BlockPos moveAboveSolid(const BlockPos& pos, const boost::shared_ptr<PathfinderMob>& mob);
    bool isWaterDestination(const BlockPos& pos, const boost::shared_ptr<PathfinderMob>& mob);

    Vec3* generateRandomPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, const Vec3* direction, bool allowWater, double maxAngleDelta, int wanderingQuadrant);
    Vec3* generateRandomPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, const Vec3* direction, bool allowWater, int wanderingQuadrant);
    Vec3* generateRandomPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, const Vec3* direction, int wanderingQuadrant);
}

#endif
