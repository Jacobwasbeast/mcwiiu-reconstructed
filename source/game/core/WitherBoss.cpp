#include "game/core/WitherBoss.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/block/Block.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

const unsigned long long WitherBoss::ENTITY_TYPE = 0x5D540047ULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGETS[3] = { NULL, NULL, NULL };
const EntityDataAccessor<int>* WitherBoss::DATA_INVULNERABLE_TICKS = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_AERIAL_ATTACK = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_CHARGE_ATTACK = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_AGGRESSIVE = NULL;

WitherBoss::WitherBoss(Level* level)
    : Monster(level), mShieldHealth(0), mPhase(0), mSpawningFrames(0), mMaxHealth(300),
      mLastHealthInterval(0), mDyingFrames(0), mOverlayAlpha(0.0f), mSwellAmount(0.0f),
      mOldSwellAmount(0.0f), mAlternativeTargets{0, 0, 0}, mHeadYRot{0.0f, 0.0f, 0.0f},
      mHeadXRot{0.0f, 0.0f, 0.0f}, mAerialAttack(false), mAggressive(false), mBossEvent(NULL)
{
    setSize(0.9f, 3.5f);
    mFireImmune = true;
    mExperienceReward = 50;
    mPhase = 0;
    mShieldHealth = 0;
    defineSynchedData();
    registerAttributes();
    registerGoals();
}
WitherBoss::~WitherBoss() = default;
void WitherBoss::create(Level* level) { new WitherBoss(level); }
void WitherBoss::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer, ENTITY_TYPE); }
void WitherBoss::staticCtor()
{
    for (int index = 0; index < 3; ++index)
        DATA_TARGETS[index] = SynchedEntityData::defineId<int>(ENTITY_TYPE, ENTITY_TYPE_MONSTER,
                                                              EntityDataSerializers::INT);
    DATA_INVULNERABLE_TICKS = SynchedEntityData::defineId<int>(ENTITY_TYPE, ENTITY_TYPE_MONSTER,
                                                               EntityDataSerializers::INT);
    DATA_AERIAL_ATTACK = SynchedEntityData::defineId<bool>(ENTITY_TYPE, ENTITY_TYPE_MONSTER,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_CHARGE_ATTACK = SynchedEntityData::defineId<bool>(ENTITY_TYPE, ENTITY_TYPE_MONSTER,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_AGGRESSIVE = SynchedEntityData::defineId<bool>(ENTITY_TYPE, ENTITY_TYPE_MONSTER,
                                                        EntityDataSerializers::BOOLEAN);
}
bool WitherBoss::isAggressive() const { return mEntityData->get(DATA_AGGRESSIVE); }
void WitherBoss::setAggressive(bool aggressive) { mEntityData->set(DATA_AGGRESSIVE, aggressive, true); }
bool WitherBoss::canDestroy(const Block* block)
{
    return block != NULL && block != Blocks::bedrock && block != Blocks::end_portal &&
           block != Blocks::end_portal_frame && block != Blocks::command_block &&
           block != Blocks::repeating_command_block && block != Blocks::chain_command_block &&
           block != Blocks::BARRIER && block != Blocks::STRUCTURE_BLOCK &&
           block != Blocks::structure_void;
}
void WitherBoss::defineSynchedData()
{
    Monster::defineSynchedData();
    for (int index = 0; index < 3; ++index)
        mEntityData->createDateItem(DATA_TARGETS[index], getId());
    mEntityData->createDateItem(DATA_INVULNERABLE_TICKS, 0);
    mEntityData->createDateItem(DATA_AERIAL_ATTACK, false);
    mEntityData->createDateItem(DATA_CHARGE_ATTACK, false);
    mEntityData->createDateItem(DATA_AGGRESSIVE, false);
}
void WitherBoss::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(300.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(70.0);
}
void WitherBoss::setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible)
{
    Entity::setCustomName(name, displayName, visible);
    if (mBossEvent != NULL)
        mBossEvent->setName(displayName);
}
PathNavigation* WitherBoss::createNavigation(Level* level)
{
    return new FlyingPathNavigation(this, level);
}
int WitherBoss::getLightColor() { return 0xF000F0; }
EMobType WitherBoss::getMobType() { return MOB_TYPE_UNDEAD; }
void WitherBoss::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Monster::startSeenByPlayer(player);
}
void WitherBoss::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Monster::stopSeenByPlayer(player);
}
void WitherBoss::getPlayerParty()
{
    if (mBossEvent != NULL)
        mBossEvent->getPartyPlayers();
}
float WitherBoss::getHeadYRot(int index) const
{
    return index >= 0 && index < 3 ? mHeadYRot[index] : 0.0f;
}
float WitherBoss::getHeadXRot(int index) const
{
    return index >= 0 && index < 3 ? mHeadXRot[index] : 0.0f;
}
float WitherBoss::rotlerp(float from, float to, float max)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > max)
        delta = max;
    if (delta < -max)
        delta = -max;
    return from + delta;
}
void WitherBoss::setAerialAttack(bool attacking) { mEntityData->set(DATA_AERIAL_ATTACK, attacking, true); }
bool WitherBoss::getAerialAttack() const { return mEntityData->get(DATA_AERIAL_ATTACK); }
void WitherBoss::addAdditonalSaveData(CompoundTag* tag)
{
    Monster::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"AirAttack"), getAerialAttack());
    tag->putInt(std::wstring(L"Invul"), getInvulnerableTicks());
    tag->putInt(std::wstring(L"ShieldHealth"), mShieldHealth);
    tag->putInt(std::wstring(L"Phase"), mPhase);
    tag->putInt(std::wstring(L"SpawningFrames"), mSpawningFrames);
    tag->putInt(std::wstring(L"maxHealth"), mMaxHealth);
    tag->putInt(std::wstring(L"lastHealthInterval"), mLastHealthInterval);
    tag->putInt(std::wstring(L"dyingFrames"), mDyingFrames);
    tag->putFloat(std::wstring(L"overlayAlpha"), mOverlayAlpha);
    tag->putFloat(std::wstring(L"swellAmount"), mSwellAmount);
    tag->putFloat(std::wstring(L"oldSwellAmount"), mOldSwellAmount);
}
int WitherBoss::getInvulnerableTicks() const { return mEntityData->get(DATA_INVULNERABLE_TICKS); }
void WitherBoss::setInvulnerableTicks(int ticks) { mEntityData->set(DATA_INVULNERABLE_TICKS, ticks, true); }
void WitherBoss::readAdditionalSaveData(CompoundTag* tag)
{
    Monster::readAdditionalSaveData(tag);
    setInvulnerableTicks(tag->getInt(std::wstring(L"Invul")));
    if (tag->contains(std::wstring(L"AirAttack")))
        setAerialAttack(tag->getBoolean(std::wstring(L"AirAttack")));
    if (tag->contains(std::wstring(L"ShieldHealth"))) mShieldHealth = tag->getInt(std::wstring(L"ShieldHealth"));
    if (tag->contains(std::wstring(L"Phase"))) mPhase = tag->getInt(std::wstring(L"Phase"));
    if (tag->contains(std::wstring(L"SpawningFrames"))) mSpawningFrames = tag->getInt(std::wstring(L"SpawningFrames"));
    if (tag->contains(std::wstring(L"maxHealth"))) mMaxHealth = tag->getInt(std::wstring(L"maxHealth"));
    if (tag->contains(std::wstring(L"lastHealthInterval"))) mLastHealthInterval = tag->getInt(std::wstring(L"lastHealthInterval"));
    if (tag->contains(std::wstring(L"dyingFrames"))) mDyingFrames = tag->getInt(std::wstring(L"dyingFrames"));
    if (tag->contains(std::wstring(L"overlayAlpha"))) mOverlayAlpha = tag->getFloat(std::wstring(L"overlayAlpha"));
    if (tag->contains(std::wstring(L"swellAmount"))) mSwellAmount = tag->getFloat(std::wstring(L"swellAmount"));
    if (tag->contains(std::wstring(L"oldSwellAmount"))) mOldSwellAmount = tag->getFloat(std::wstring(L"oldSwellAmount"));
}
void WitherBoss::readAdditionContructionTag(CompoundTag* tag)
{
    if (tag->contains(std::wstring(L"AirAttack")))
        setAerialAttack(tag->getBoolean(std::wstring(L"AirAttack")));
    if (tag->contains(std::wstring(L"Invul")))
        setInvulnerableTicks(tag->getInt(std::wstring(L"Invul")));
}
int WitherBoss::getAdditionContructionTag() const { return getInvulnerableTicks(); }
bool WitherBoss::isPowered() const { return getInvulnerableTicks() > 0; }
bool WitherBoss::IsChargeAttacking() { return mEntityData->get(DATA_CHARGE_ATTACK); }
bool WitherBoss::canShoot() const { return getInvulnerableTicks() == 0; }
void WitherBoss::aiStep()
{
    if (getInvulnerableTicks() > 0)
        setInvulnerableTicks(getInvulnerableTicks() - 1);
    if (mSpawningFrames > 0)
        --mSpawningFrames;
    mOldSwellAmount = mSwellAmount;
    mSwellAmount = getAerialAttack() ? 1.0f : 0.0f;
    customServerAiStep();
    setAggressive(getInvulnerableTicks() > 0 || getTarget() != 0);
    Monster::aiStep();
}
void WitherBoss::makeInvulnerable() { setInvulnerableTicks(220); }
void WitherBoss::makeStuckInWeb() { mXd = 0.0; mYd = 0.0; mZd = 0.0; }
int WitherBoss::getArmorValue() const { return 4; }
bool WitherBoss::hurt(const DamageSource* source, float damage)
{
    if (getInvulnerableTicks() > 0)
        return false;
    if (mShieldHealth > 0)
        mShieldHealth -= static_cast<int>(damage);
    return Monster::hurt(source, damage);
}
bool WitherBoss::isPickable() { return true; }
void WitherBoss::causeFallDamage(float fallDistance, float multiplier)
{
    LivingEntity::causeFallDamage(fallDistance, multiplier);
}
void WitherBoss::heal(float amount) { LivingEntity::heal(amount); }
bool WitherBoss::canFly() const { return true; }
void WitherBoss::registerGoals()
{
    mGoalSelector.addGoal(0, new WitherDoNothingGoal(this), true);
    setAerialAttack(false);
}
const SoundEvent* WitherBoss::getAmbientSound() { return SoundEvents::WITHER_AMBIENT; }
const SoundEvent* WitherBoss::getHurtSound(const DamageSource* source) { return Monster::getHurtSound(source); }
const SoundEvent* WitherBoss::getDeathSound() { return Monster::getDeathSound(); }
void WitherBoss::checkDespawn() { Monster::checkDespawn(); }
bool WitherBoss::canRide(boost::shared_ptr<Entity>) { return false; }
bool WitherBoss::canChangeDimensions() { return true; }
double WitherBoss::getHeadX(int head) const
{
    if (head < 1)
        return mX;
    const float angle = (mYRot + static_cast<float>((head - 1) * 180 - 180)) * 0.017453292384f;
    return static_cast<double>(Mth::cos(angle)) * 1.3 + mX;
}
double WitherBoss::getHeadY(int head) const { return mY + (head < 1 ? 3.0 : 2.2); }
double WitherBoss::getHeadZ(int head) const
{
    if (head < 1)
        return mZ;
    const float angle = (mYRot + static_cast<float>((head - 1) * 180 - 180)) * 0.017453292384f;
    return static_cast<double>(Mth::sin(angle)) * 1.3 + mZ;
}
float WitherBoss::getMaxHealth() const { return 300.0f; }
float WitherBoss::getHealth() { return LivingEntity::getHealth(); }
unsigned long long WitherBoss::GetType() { return ENTITY_TYPE; }
std::wstring WitherBoss::getDisplayName() const { return std::wstring(L"Wither"); }
void WitherBoss::performRangedAttack(boost::shared_ptr<LivingEntity> target, float power)
{
    (void)power;
    performRangedAttack(0, target);
}
void WitherBoss::performRangedAttack(int head, boost::shared_ptr<LivingEntity> target)
{
    if (target)
        target->hurt(DamageSource::CreateMobAttack(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this())), 5.0f);
}
int WitherBoss::getAlternativeTarget(int head) const
{
    return head >= 0 && head < 3 ? mAlternativeTargets[head] : 0;
}
void WitherBoss::setAlternativeTarget(int head, int target)
{
    const int selfId = getId();
    if (head == 0)
    {
        if (target != selfId && mAlternativeTargets[1] == target)
            mAlternativeTargets[1] = selfId;
        if (target != selfId && mAlternativeTargets[2] == target)
            mAlternativeTargets[2] = selfId;
    }
    else if (target != selfId && (mAlternativeTargets[0] == target ||
                                  mAlternativeTargets[1] == target ||
                                  mAlternativeTargets[2] == target))
    {
        return;
    }
    if (head >= 0 && head < 3)
        mAlternativeTargets[head] = target;
}
void WitherBoss::generateParticles() { mOverlayAlpha = getAerialAttack() ? 1.0f : 0.0f; }
void WitherBoss::changePhase() { mPhase = mPhase == 0 ? 1 : 0; }
bool WitherBoss::destroyBlocks(Level* level, AABB* area, int radius)
{
    if (level == NULL || area == NULL || !level->getGameRules()->getBoolean(1))
        return false;

    bool destroyedAny = false;
    const int minX = static_cast<int>(area->mMinX) - radius;
    const int maxX = static_cast<int>(area->mMaxX) + radius;
    const int minY = static_cast<int>(area->mMinY) - radius;
    const int maxY = static_cast<int>(area->mMaxY) + radius;
    const int minZ = static_cast<int>(area->mMinZ) - radius;
    const int maxZ = static_cast<int>(area->mMaxZ) + radius;
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                const BlockPos pos(x, y, z);
                const BlockState* state = level->getBlockState(pos);
                if (state == NULL || !canDestroy(state->getBlock()))
                    continue;
                if (level->destroyBlock(pos, true))
                    destroyedAny = true;
            }
        }
    }
    return destroyedAny;
}
void WitherBoss::customServerAiStep() { generateParticles(); }
void WitherBoss::dropDeathLoot(bool hitByPlayer, int lootingLevel) { Monster::dropAllDeathLoot(hitByPlayer, lootingLevel, NULL); }
float WitherBoss::getOverlayAlpha() const { return mOverlayAlpha; }
float WitherBoss::getSwellAmount(float partialTick) const { return mOldSwellAmount + (mSwellAmount - mOldSwellAmount) * partialTick; }
int WitherBoss::getTarget() const { return mAlternativeTargets[0]; }
bool WitherBoss::IsSpawning() const { return mSpawningFrames > 0; }
void WitherBoss::setWantsToMove(bool wantsToMove) { mAerialAttack = wantsToMove; }
bool WitherBoss::wantsToMove() const { return mAerialAttack; }
void WitherBoss::setIsPathing(bool pathing) { mAggressive = pathing; }
void WitherBoss::setShotDelay(int delay) { mLastHealthInterval = delay; }
int WitherBoss::getDimension() const { return mPhase; }
WitherBoss::WitherDoNothingGoal::WitherDoNothingGoal(WitherBoss* boss) : Goal(), mBoss(boss)
{
    setRequiredControlFlags(7);
}
WitherBoss::WitherDoNothingGoal::~WitherDoNothingGoal() = default;
bool WitherBoss::WitherDoNothingGoal::canUse() { return mBoss->getInvulnerableTicks() > 0; }
std::wstring WitherBoss::WitherDoNothingGoal::getName()
{
    std::wstring name;
    name.assign(L"WitherDoNothingGoal");
    return name;
}
