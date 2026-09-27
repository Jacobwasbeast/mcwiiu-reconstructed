#include "game/core/Salmon.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/RiverFollowingGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

static const EntityDataAccessor<unsigned char>* DATA_ID_SCALE = NULL;

Salmon::Salmon(Level* level)
    : Fish(level, true)
{
    fjDerivedCtorCalls();
    SalmonWeightedRandomize();
    mSwimAmplitude = 0.01f;
    mSwimFrequency = 0.02f;
    mWaterValueC = 12.0f;
}

Salmon::~Salmon()
{
}

void Salmon::create(Level* level)
{
    new Salmon(level);
}

void Salmon::staticCtor()
{
    DATA_ID_SCALE = SynchedEntityData::defineId(EntityType::SALMON, EntityType::WATER_ANIMAL,
                                                 EntityDataSerializers::BYTE);
}

void Salmon::setSalmonScale(SalmonScale scale)
{
    if (mEntityData != NULL && DATA_ID_SCALE != NULL)
        mEntityData->set(DATA_ID_SCALE, static_cast<unsigned char>(scale), true);
}

void Salmon::SalmonWeightedRandomize()
{
    int value = mRandom->nextInt(95);
    if (value < 30)
        setSalmonScale(SMALL);
    else if (value < 80)
        setSalmonScale(MEDIUM);
    else
        setSalmonScale(LARGE);
}

Salmon::SalmonScale Salmon::getSalmonScale()
{
    if (mEntityData == NULL || DATA_ID_SCALE == NULL)
        return SMALL;
    return static_cast<SalmonScale>(mEntityData->get(DATA_ID_SCALE));
}

void Salmon::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"SalmonScale", static_cast<int>(getSalmonScale()));
}

void Salmon::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setSalmonScale(static_cast<SalmonScale>(tag->getInt(L"SalmonScale")));
}

void Salmon::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_ID_SCALE == NULL || accessor != DATA_ID_SCALE)
        return;

    SalmonScale scale = getSalmonScale();
    if (scale == SMALL)
    {
        setSize(0.5f, 0.5f);
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(2.0);
    }
    else if (scale == MEDIUM)
    {
        setSize(1.0f, 1.0f);
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(3.0);
    }
    else if (scale == LARGE)
    {
        setSize(1.5f, 1.5f);
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(5.0);
    }
}

void Salmon::defineSynchedData()
{
    Mob::defineSynchedData();
    if (mEntityData != NULL && DATA_ID_SCALE != NULL)
        mEntityData->createDateItem(DATA_ID_SCALE, static_cast<unsigned char>(SMALL));
}

void Salmon::registerGoals()
{
    Fish::registerGoals();
    mGoalSelector.addGoal(3, new RiverFollowingGoal(this, 2.0f, 1.2f), true);
}

void Salmon::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(3.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.2);
}

ResourceLocation* Salmon::getDefaultLootTable()
{
    if (getSalmonScale() == LARGE)
        return &BuiltInLootTables::SALMON_LARGE;
    return &BuiltInLootTables::SALMON_NORMAL;
}

unsigned long long Salmon::GetType()
{
    return EntityType::SALMON;
}
