#include "game/core/Spider.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/ai/WallClimberNavigation.hpp"
#include "game/util/Random.hpp"

static const unsigned long long ENTITY_TYPE_SPIDER = 0x5D560040ULL;
static const EntityDataAccessor<unsigned char>* SPIDER_DATA_FLAGS = NULL;

Spider::Spider(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    setSize(1.4f, 0.9f);
    mBlocksBuilding = true;
}

Spider::Spider(Level* level, bool callDerivedConstructor) : Monster(level)
{
    if (callDerivedConstructor) fjDerivedCtorCalls();
    setSize(1.4f, 0.9f);
    mBlocksBuilding = true;
}

Spider::~Spider()
{
}

void Spider::create(Level* level)
{
    new Spider(level);
}

void Spider::staticCtor()
{
    SPIDER_DATA_FLAGS = SynchedEntityData::defineId(ENTITY_TYPE_SPIDER, 0,
                                                     EntityDataSerializers::BYTE);
}

void Spider::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_SPIDER);
}

void Spider::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(3, new LeapAtTargetGoal(this, 0.4f), true);
    mGoalSelector.addGoal(4, new SpiderAttackGoal(this), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 0.8), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);
    mGoalSelector.addGoal(6, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new SpiderTargetGoal(this, typeid(Mob)), true);
    mTargetSelector.addGoal(3, new SpiderTargetGoal(this, typeid(Mob)), true);
}

double Spider::getRideHeight()
{
    return (double)(mBbHeight * 0.5f);
}

PathNavigation* Spider::createNavigation(Level* level)
{
    return new WallClimberNavigation(this, level);
}

void Spider::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(SPIDER_DATA_FLAGS, (unsigned char)0);
}

void Spider::tick()
{
    Monster::tick();
    if (!mLevel->mIsClientSide) setClimbing(isClimbing());
}

void Spider::registerAttributes()
{
    Monster::registerAttributes();
    AttributeInstance* health = getAttribute(NULL);
    if (health != NULL) health->setBaseValue(16.0);
    AttributeInstance* speed = getAttribute(NULL);
    if (speed != NULL) speed->setBaseValue(0.3);
}

const SoundEvent* Spider::getAmbientSound() { return SoundEvents::SPIDER_AMBIENT; }
const SoundEvent* Spider::getHurtSound(const DamageSource*) { return SoundEvents::SPIDER_HURT; }
const SoundEvent* Spider::getDeathSound() { return SoundEvents::SPIDER_DEATH; }

void Spider::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::SPIDER_STEP, 0.15f, 1.0f);
}

const void* Spider::getDefaultLootTable() { return &BuiltInLootTables::SPIDER; }
bool Spider::onLadder() { return isClimbing(); }
void Spider::makeStuckInWeb() { }
EMobType Spider::getMobType() { return MOB_TYPE_ARTHROPOD; }

bool Spider::canBeAffected(MobEffectInstance* effect)
{
    if (effect->getEffect() == NULL) return false;
    return LivingEntity::canBeAffected(effect);
}

bool Spider::isClimbing()
{
    return (getEntityData()->get(SPIDER_DATA_FLAGS) & 1) != 0;
}

void Spider::setClimbing(bool climbing)
{
    unsigned char flags = getEntityData()->get(SPIDER_DATA_FLAGS);
    flags = climbing ? (unsigned char)(flags | 1) : (unsigned char)(flags & 0xfe);
    getEntityData()->set(SPIDER_DATA_FLAGS, flags, true);
}

MobGroupData* Spider::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring&)
{
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

double Spider::getEyeHeight() { return 0.65; }
unsigned long long Spider::GetType() { return ENTITY_TYPE_SPIDER; }

Spider::SpiderAttackGoal::SpiderAttackGoal(Spider* spider)
    : MeleeAttackGoal(spider, 1.0, true)
{
}

Spider::SpiderAttackGoal::~SpiderAttackGoal() { }

bool Spider::SpiderAttackGoal::canContinueToUse()
{
    if (mMob == NULL || mMob->getNavigation() == NULL) return false;
    if (mMob->getNavigation()->isDone() && mMob->getTarget() == NULL) return false;
    return MeleeAttackGoal::canContinueToUse();
}

double Spider::SpiderAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return (double)(target->mBbWidth * target->mBbWidth + 4.0f);
}

Spider::SpiderEffectsGroupData::SpiderEffectsGroupData() : mEffect(0) { }
Spider::SpiderEffectsGroupData::~SpiderEffectsGroupData() { }

unsigned long long Spider::SpiderEffectsGroupData::setRandomEffect(Random* random)
{
    int effect = random->nextInt(5);
    mEffect = effect < 2 ? 0 : effect == 2 ? 1 : effect == 3 ? 2 : 3;
    return 0;
}

Spider::SpiderTargetGoal::SpiderTargetGoal(Spider* spider, const std::type_info& targetType)
    : NearestAttackableTargetGoal(spider, targetType, true, false, false)
{
}

Spider::SpiderTargetGoal::~SpiderTargetGoal() { }

bool Spider::SpiderTargetGoal::canUse()
{
    return mMob->getBrightness() < 0.5 && NearestAttackableTargetGoal::canUse();
}
