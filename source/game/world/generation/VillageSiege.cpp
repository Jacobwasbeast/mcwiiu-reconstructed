#include "game/world/generation/VillageSiege.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/generation/Player.hpp"

VillageSiege::VillageSiege(Level* level)
    : mLevel(level), mHasSetupSiege(false), mSiegeState(-1), mSiegeCount(0), mNextSpawnTime(0), mVillage(), mSpawnPos()
{
}

Vec3* VillageSiege::findRandomSpawnPos(const BlockPos& pos)
{
    boost::shared_ptr<Village> village = mVillage.lock();
    if (!village)
        return NULL;

    for (int attempt = 0; attempt < 10; ++attempt)
    {
        BlockPos candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8,
                                        mLevel->mRandom->nextInt(6) - 3,
                                        mLevel->mRandom->nextInt(16) - 8);
        if (village->isInside(candidate) &&
            NaturalMobSpawner::isSpawnPositionOk(Mob::SPAWN_PLACEMENT_TYPE_DEFAULT,
                                                  mLevel, candidate, 10, 10))
        {
            return Vec3::newTemp((double)candidate.x,
                                  (double)candidate.y,
                                  (double)candidate.z);
        }
    }
    return NULL;
}

bool VillageSiege::tryToSetupSiege()
{
    BlockPos playerPos;
    std::vector<boost::shared_ptr<Player> >* players = mLevel->getPlayers(0, NULL);

    std::vector<boost::shared_ptr<Player> >::iterator it = players->begin();
    for (; it != players->end(); ++it)
    {
        if (!*it || (*it)->isSpectator())
            continue;

        playerPos = BlockPos(boost::static_pointer_cast<Entity>(*it));
        Villages* villages = (Villages*)mLevel->getVillages().get();
        boost::shared_ptr<Village> village = villages->getClosestVillage(playerPos, 1);
        if (!village)
            continue;
        if (village->getDoorCount() < 10 || village->getStableAge() < 20 || village->getPopulationSize() < 20)
            continue;

        const BlockPos* center = village->getCenter();
        float radius = (float)village->getRadius();
        mVillage = village;
        for (int attempt = 0; attempt < 10; ++attempt)
        {
            float angle = mLevel->mRandom->nextFloat() * 6.2831855f;
            mSpawnPos = BlockPos(center->x + (int)(Mth::cos(angle) * radius * 0.9f),
                                 center->y,
                                 center->z + (int)(Mth::sin(angle) * radius * 0.9f));
            bool insideOtherVillage = false;
            std::vector<boost::shared_ptr<Village> >* allVillages = villages->getVillages();
            std::vector<boost::shared_ptr<Village> >::iterator villageIt = allVillages->begin();
            for (; villageIt != allVillages->end(); ++villageIt)
            {
                if (*villageIt != village && (*villageIt)->isInside(mSpawnPos))
                {
                    insideOtherVillage = true;
                    break;
                }
            }
            if (!insideOtherVillage && findRandomSpawnPos(mSpawnPos))
            {
                mNextSpawnTime = 0;
                mSiegeCount = 20;
                return true;
            }
        }
    }
    return false;
}

bool VillageSiege::trySpawn()
{
    Vec3* spawn = findRandomSpawnPos(mSpawnPos);
    if (!spawn)
        return false;

    boost::shared_ptr<Entity> entity = EntityIO::newEntity(ENTITY_TYPE_ZOMBIE, mLevel);
    DifficultyInstance difficulty = mLevel->getCurrentDifficultyAt(BlockPos(entity));
    boost::static_pointer_cast<Mob>(entity)->finalizeMobSpawn(difficulty, NULL, std::wstring());
    entity->moveTo(spawn->x, spawn->y, spawn->z,
                   mLevel->mRandom->nextFloat() * 360.0f, 0.0f);
    mLevel->addEntity(entity);
    return true;
}

void VillageSiege::tick()
{
    if (mLevel->isDay())
    {
        mSiegeState = 0;
        return;
    }
    if (mSiegeState == 2)
        return;

    if (mSiegeState == 0)
    {
        float time = mLevel->getTimeOfDay(0.0f);
        if (time < 0.5f || time > 0.501f)
            return;
        mSiegeState = mLevel->mRandom->nextInt(10) == 0 ? 1 : 2;
        mHasSetupSiege = false;
        if (mSiegeState == 2)
            return;
    }
    else if (!mHasSetupSiege)
    {
        if (!tryToSetupSiege())
            return;
        mHasSetupSiege = true;
    }

    if (mNextSpawnTime > 0)
    {
        --mNextSpawnTime;
        return;
    }
    mNextSpawnTime = 2;
    if (mSiegeCount < 1)
    {
        mSiegeState = 2;
        return;
    }
    trySpawn();
    --mSiegeCount;
}
