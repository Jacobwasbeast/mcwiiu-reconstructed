#include "game/core/CaveSpider.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
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

    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(target);
    if (living)
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
            living->addEffect(new MobEffectInstance(MobEffects::POISON, duration * 20, 0),
                               boost::shared_ptr<Entity>());
        }
    }

    return true;
}

MobGroupData* CaveSpider::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name)
{
    if (mRandom->nextInt(100) != 0)
    {
        return data;
    }

    BlockPos pos((int)mX, (int)mY, (int)mZ);
    if (mLevel->canSeeSky(pos) && !mLevel->isDay())
    {
        mRandom->rollPercentageChance(80);
    }

    (void)difficulty;
    (void)name;
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
