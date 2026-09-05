#include "game/world/generation/VillageSiege.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

class NaturalMobSpawner
{
public:
    static bool isSpawnPositionOk(int placementType, Level* level, const BlockPos& pos, int xSize, int zSize);
};

VillageSiege::VillageSiege(Level* level)
    : mLevel(level), mHasSetupSiege(false), mSiegeState(-1), mSiegeCount(0), mNextSpawnTime(0), mVillage(), mSpawnPos()
{
}

Vec3* VillageSiege::findRandomSpawnPos(const BlockPos& pos)
{
    if (!mVillage)
        return NULL;

    for (int attempt = 0; attempt < 5; ++attempt)
    {
        BlockPos candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8,
                                        mLevel->mRandom->nextInt(6) - 3,
                                        mLevel->mRandom->nextInt(16) - 8);
        if (mVillage->isInside(candidate) &&
            NaturalMobSpawner::isSpawnPositionOk(0, mLevel, candidate, 10, 10))
        {
            return Vec3::newTemp((double)candidate.x - 0.5,
                                  (double)candidate.y,
                                  (double)candidate.z - 0.5);
        }
    }
    return NULL;
}

int VillageSiege::tryToSetupSiege()
{
    BlockPos playerPos;
    std::vector<boost::shared_ptr<Player> >* players = mLevel->getPlayers(0, NULL);
    if (!players)
        return 0;

    std::vector<boost::shared_ptr<Player> >::iterator it = players->begin();
    for (; it != players->end(); ++it)
    {
        if (!*it)
            continue;

        playerPos = BlockPos(boost::static_pointer_cast<Entity>(*it));
        Villages* villages = mLevel->getVillages();
        boost::shared_ptr<Village> village = villages->getClosestVillage(playerPos, 1);
        if (!village)
            continue;
        if (village->getDoorCount() < 10 || village->getStableAge() < 20 || village->getPopulationSize() < 1)
            continue;

        const BlockPos* center = village->getCenter();
        int radius = (int)((float)village->getRadius() * 0.9f);
        mVillage = village;
        for (int attempt = 0; attempt < 10; ++attempt)
        {
            float angle = mLevel->mRandom->nextFloat() * 6.2831855f;
            mSpawnPos = BlockPos(center->x + (int)((double)Mth::cos(angle) * radius),
                                 center->y,
                                 center->z + (int)((double)Mth::sin(angle) * radius));
            if (!villages->getClosestVillage(mSpawnPos, 1) && findRandomSpawnPos(mSpawnPos))
            {
                mHasSetupSiege = false;
                mSiegeCount = 20;
                return 1;
            }
        }
    }
    return 0;
}

int VillageSiege::trySpawn()
{
    Vec3* spawn = findRandomSpawnPos(mSpawnPos);
    if (!spawn)
        return 0;

    boost::shared_ptr<Zombie> zombie(new Zombie(mLevel));
    zombie->absMoveTo(spawn->x, spawn->y, spawn->z,
                      mLevel->mRandom->nextFloat() * 360.0f, 0.0f);
    DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(mSpawnPos);
    zombie->finalizeMobSpawn(difficulty, NULL, std::wstring());
    mLevel->addEntity(boost::static_pointer_cast<Entity>(zombie));
    return 1;
}

unsigned long long VillageSiege::tick()
{
    if (mLevel->isDay())
    {
        mSiegeState = 0;
        return 0;
    }
    if (mSiegeState == 2)
        return 0;

    if (mSiegeState == 0)
    {
        float time = mLevel->getTimeOfDay(0.0f);
        if (time < 0.5f || time > 0.501f)
            return 0;
        mSiegeState = mLevel->getRandom()->nextInt(10) == 0 ? 1 : 2;
        mHasSetupSiege = false;
        if (mSiegeState == 2)
            return 0;
    }
    else if (!mHasSetupSiege)
    {
        if (!tryToSetupSiege())
            return 0;
        mHasSetupSiege = true;
    }

    if (mNextSpawnTime > 0)
    {
        --mNextSpawnTime;
        return 0;
    }
    mNextSpawnTime = 2;
    if (mSiegeCount < 1)
    {
        mSiegeState = 2;
        return 0;
    }
    trySpawn();
    --mSiegeCount;
    return 0;
}
