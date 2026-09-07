#include "game/core/CaveSpider.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Stray.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"

static const unsigned long long ENTITY_TYPE_CAVE_SPIDER = 0x5D560041ULL;

CaveSpider::CaveSpider(Level* level) : Spider(level, false)
{
    fjDerivedCtorCalls();
    setSize(0.7f, 0.5f);
}

CaveSpider::~CaveSpider()
{
}

void CaveSpider::create(Level* level)
{
    new CaveSpider(level);
}

void CaveSpider::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_CAVE_SPIDER);
}

void CaveSpider::registerAttributes()
{
    Spider::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(12.0);
}

bool CaveSpider::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (!Monster::doHurtTarget(target))
    {
        return false;
    }

    if ((target->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        int duration = 0;
        const Difficulty* difficulty = mLevel->getDifficulty();
        if (difficulty == Difficulty::NORMAL)
        {
            duration = 7;
        }
        else if (difficulty == Difficulty::HARD)
        {
            duration = 15;
        }

        if (duration > 0)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(target);
            living->addEffect(new MobEffectInstance(MobEffects::POISON, duration * 20, 0),
                               boost::shared_ptr<Entity>());
        }
    }

    return true;
}

MobGroupData* CaveSpider::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring&)
{
    if (mLevel->mRandom->nextInt(100) != 0)
    {
        return data;
    }

    BlockPos pos(mX, mY, mZ);
    Biome* biome = NULL;
    boost::shared_ptr<AbstractSkeleton> rider;

    if (mLevel->canSeeSky(pos) && !mLevel->isDay() &&
        !mLevel->mRandom->rollPercentageChance(80))
    {
        biome = mLevel->getBiome(pos);
        if (biome == Biome::ICE_PLAINS || biome == Biome::ICE_MOUNTAINS ||
            biome == Biome::ICE_PLAINS_SPIKES || biome == Biome::FROZEN_RIVER ||
            biome == Biome::COLD_BEACH || biome == Biome::FROZEN_OCEAN ||
            biome == Biome::DEEP_FROZEN_OCEAN)
        {
            rider.reset(new Stray(mLevel));
        }
    }

    if (!rider)
    {
        if (mLevel->mRandom->rollPercentageChance(80) &&
            mLevel->getBiome(pos) == Biome::HELL)
        {
            rider.reset(new WitherSkeleton(mLevel));
        }
        else
        {
            rider.reset(new Skeleton(mLevel));
        }
    }

    rider->moveTo(mX, mY, mZ, mYRot, 0.0f);
    rider->finalizeMobSpawn(difficulty, NULL, std::wstring());
    mLevel->addEntity(rider);
    Entity* riderEntity = rider.get();
    riderEntity->startRiding(shared_from_this());

    return data;
}

double CaveSpider::getEyeHeight()
{
    return 0.45;
}

const void* CaveSpider::getDefaultLootTable()
{
    return &BuiltInLootTables::CAVE_SPIDER;
}

unsigned long long CaveSpider::GetType()
{
    return ENTITY_TYPE_CAVE_SPIDER;
}
