#ifndef GAME_WORLD_GENERATION_VILLAGESIEGE_HPP
#define GAME_WORLD_GENERATION_VILLAGESIEGE_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"

class Level;
class Village;
class Vec3;

class VillageSiege
{
public:
    VillageSiege(Level* level);

    Vec3* findRandomSpawnPos(const BlockPos& pos);
    int tryToSetupSiege();
    int trySpawn();
    unsigned long long tick();

private:
    Level* mLevel;
    bool mHasSetupSiege;
    int mSiegeState;
    int mSiegeCount;
    int mNextSpawnTime;
    boost::shared_ptr<Village> mVillage;
    BlockPos mSpawnPos;
};

#endif
