#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/world/LevelEvent.hpp"

#include <algorithm>

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/WeighedRandom.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"

BaseMobSpawner::BaseMobSpawner()
    : mDelay(20), mSpawnPotentials(), mNextSpawnData(new SpawnData()), mSpin(0.0), moSpin(0.0),
      mMinSpawnDelay(200), mMaxSpawnDelay(800), mSpawnCount(4), mDisplayEntity(),
      mMaxNearbyEntities(6), mRequiredPlayerRange(16), mSpawnRange(4)
{
}

BaseMobSpawner::~BaseMobSpawner()
{
    std::vector<SpawnData*>::iterator it = mSpawnPotentials.begin();
    for (; it != mSpawnPotentials.end(); ++it)
    {
        if (*it != mNextSpawnData)
            delete *it;
    }
    delete mNextSpawnData;
}

ResourceLocation BaseMobSpawner::getEntityId()
{
    return ResourceLocation(mNextSpawnData->getTag()->getString(L"id"));
}

void BaseMobSpawner::setEntityId(const ResourceLocation& entityId)
{
    if (EntityIO::isValidEntityType(entityId))
        mNextSpawnData->getTag()->putString(L"id", entityId.toString());
}

bool BaseMobSpawner::isNearPlayer()
{
    Level* level = getLevel();
    if (level == NULL)
        return false;
    BlockPos pos = getPos();
    return level->hasNearPlayer(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
                                static_cast<double>(mRequiredPlayerRange));
}

bool BaseMobSpawner::hasMob()
{
    return EntityIO::isValidEntityType(getEntityId());
}

void BaseMobSpawner::tick()
{
    if (!isNearPlayer())
    {
        moSpin = mSpin;
        return;
    }
    if (!hasMob())
        return;
    Level* level = getLevel();
    if (level == NULL)
        return;
    if (level->mIsClientSide)
    {
        BlockPos pos = getPos();
        level->addParticle(PARTICLE_SMOKE, pos.x + level->mRandom->nextFloat(), pos.y + level->mRandom->nextFloat(), pos.z + level->mRandom->nextFloat(), 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, pos.x + level->mRandom->nextFloat(), pos.y + level->mRandom->nextFloat(), pos.z + level->mRandom->nextFloat(), 0.0, 0.0, 0.0, arrayWithLength<int>());
        if (mDelay > 0)
            --mDelay;
        moSpin = mSpin;
        mSpin = static_cast<double>(static_cast<int>(mSpin + 1000.0 / (mDelay + 200.0)) % 360);
        return;
    }
    if (mDelay == -1)
        delay();
    if (mDelay > 0)
    {
        --mDelay;
        return;
    }
    BlockPos origin = getPos();
    bool spawned = false;
    for (int i = 0; i < mSpawnCount; ++i)
    {
        ListTag* position = mNextSpawnData->getTag()->fjSafeGetListTag(L"Pos", 6);
        double x;
        double y;
        double z;
        if (position == NULL || position->size() < 1)
        {
            x = static_cast<double>(origin.x) + level->mRandom->nextDouble() * 2.0 * mSpawnRange - mSpawnRange;
        }
        else
        {
            x = position->getDouble(0);
        }
        if (position == NULL || position->size() < 2)
        {
            y = static_cast<double>(origin.y) + level->mRandom->nextInt(3) - 1;
        }
        else
        {
            y = position->getDouble(1);
        }
        if (position == NULL || position->size() < 3)
        {
            z = static_cast<double>(origin.z) + level->mRandom->nextDouble() * 2.0 * mSpawnRange - mSpawnRange;
        }
        else
        {
            z = position->getDouble(2);
        }
        boost::shared_ptr<Entity> entity = OldChunkStorage::loadEntity(mNextSpawnData->getTag(), level, x, y, z, false);
        if (entity)
        {
            AABB* box = AABB::newTemp(origin)->grow(static_cast<double>(mSpawnRange),
                                                      static_cast<double>(mSpawnRange),
                                                      static_cast<double>(mSpawnRange));
            std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(*entity), box);
            int nearby = entities == NULL ? 0 : static_cast<int>(entities->size());
            delete entities;
            if (nearby < mMaxNearbyEntities)
            {
                if (!level->canCreateMore(entity->GetType(), Level::eSpawnType_Egg))
                {
                    delay();
                    return;
                }
                boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(entity);
                if (mob)
                {
                    DifficultyInstance difficulty = level->getCurrentDifficultyAt(origin);
                    mob->finalizeMobSpawn(difficulty, NULL, std::wstring());
                }
                OldChunkStorage::addEntityToWorld(entity, level);
                level->levelEvent(LevelEvent::PARTICLES_MOBBLOCK_SPAWN, origin, 0);
                spawned = true;
            }
        }
    }
    if (spawned)
        delay();
}

void BaseMobSpawner::delay()
{
    Random* random = getLevel()->mRandom;
    mDelay = mMinSpawnDelay;
    if (mMinSpawnDelay < mMaxSpawnDelay)
        mDelay += random->nextInt(mMaxSpawnDelay - mMinSpawnDelay);
    if (!mSpawnPotentials.empty())
        setNextSpawnData(static_cast<SpawnData*>(WeighedRandom::getRandomItem(random, mSpawnPotentials.begin(), mSpawnPotentials.end())));
    broadcastEvent(1);
}

void BaseMobSpawner::load(CompoundTag* tag)
{
    mDelay = tag->getShort(L"Delay");
    mSpawnPotentials.clear();
    if (tag->contains(L"SpawnPotentials"))
    {
        ListTag* list = tag->getList(L"SpawnPotentials", Tag::TAG_COMPOUND);
        for (int i = 0; i < list->size(); ++i)
            mSpawnPotentials.push_back(new SpawnData(list->getCompound(i)));
    }
    if (tag->contains(L"SpawnData", Tag::TAG_COMPOUND))
        setNextSpawnData(new SpawnData(1, tag->getCompound(L"SpawnData")));
    else if (!mSpawnPotentials.empty())
        setNextSpawnData(static_cast<SpawnData*>(WeighedRandom::getRandomItem(getLevel()->mRandom, mSpawnPotentials.begin(), mSpawnPotentials.end())));
    if (tag->contains(L"MinSpawnDelay", Tag::TAG_ANY_NUMERIC))
    {
        mMinSpawnDelay = tag->getShort(L"MinSpawnDelay");
        mMaxSpawnDelay = tag->getShort(L"MaxSpawnDelay");
        mSpawnCount = tag->getShort(L"SpawnCount");
    }
    if (tag->contains(L"MaxNearbyEntities", Tag::TAG_ANY_NUMERIC))
    {
        mMaxNearbyEntities = tag->getShort(L"MaxNearbyEntities");
        mRequiredPlayerRange = tag->getShort(L"RequiredPlayerRange");
    }
    if (tag->contains(L"SpawnRange", Tag::TAG_ANY_NUMERIC))
        mSpawnRange = tag->getShort(L"SpawnRange");
    mDisplayEntity.reset();
}

CompoundTag* BaseMobSpawner::save(CompoundTag* tag)
{
    ResourceLocation entityId = getEntityId();
    if (!entityId.isDefault() && !entityId.getPath().empty())
    {
        tag->putShort(L"Delay", static_cast<short>(mDelay));
        tag->putShort(L"MinSpawnDelay", static_cast<short>(mMinSpawnDelay));
        tag->putShort(L"MaxSpawnDelay", static_cast<short>(mMaxSpawnDelay));
        tag->putShort(L"SpawnCount", static_cast<short>(mSpawnCount));
        tag->putShort(L"MaxNearbyEntities", static_cast<short>(mMaxNearbyEntities));
        tag->putShort(L"RequiredPlayerRange", static_cast<short>(mRequiredPlayerRange));
        tag->putShort(L"SpawnRange", static_cast<short>(mSpawnRange));
        tag->put(L"SpawnData", mNextSpawnData->getTag()->copy());
        ListTag* list = new ListTag();
        if (mSpawnPotentials.empty())
            list->add(mNextSpawnData->save());
        else
        {
            std::vector<SpawnData*>::const_iterator it = mSpawnPotentials.begin();
            for (; it != mSpawnPotentials.end(); ++it)
                list->add((*it)->save());
        }
        tag->put(L"SpawnPotentials", list);
    }
    return tag;
}

boost::shared_ptr<Entity> BaseMobSpawner::getOrCreateDisplayEntity(Level* level)
{
    if (!mDisplayEntity)
        mDisplayEntity = OldChunkStorage::loadEntity(mNextSpawnData->getTag(), getLevel(), false);
    return mDisplayEntity;
}

int BaseMobSpawner::onEventTriggered(int event)
{
    if (event == 2)
    {
        if (getLevel()->mIsClientSide)
        {
            BlockPos pos = getPos();
            getLevel()->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, pos, 0);

            Random random;
            for (int i = 0; i < 20; ++i)
            {
                float x = (float)pos.x + getLevel()->mRandom->nextFloat();
                float y = (float)pos.y + getLevel()->mRandom->nextFloat();
                float z = (float)pos.z + getLevel()->mRandom->nextFloat();
                double xd = random.nextGaussian() * 0.02;
                double yd = random.nextGaussian() * 0.02;
                double zd = random.nextGaussian() * 0.02;
                getLevel()->addParticle(PARTICLE_EXPLODE, x, y, z, xd, yd, zd, arrayWithLength<int>());
            }
        }
        return 1;
    }

    if (event == 1 && getLevel()->mIsClientSide)
    {
        mDelay = mMinSpawnDelay;
        return 1;
    }

    return 0;
}

void BaseMobSpawner::setNextSpawnData(SpawnData* data)
{
    if (std::find(mSpawnPotentials.begin(), mSpawnPotentials.end(), mNextSpawnData) == mSpawnPotentials.end())
        delete mNextSpawnData;
    mNextSpawnData = data;
}

double BaseMobSpawner::getSpin()
{
    return mSpin;
}

double BaseMobSpawner::getoSpin()
{
    return moSpin;
}
