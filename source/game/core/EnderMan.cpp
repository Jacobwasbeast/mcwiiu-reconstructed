#include "game/core/EnderMan.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static const EntityDataAccessor<const BlockState*>* DATA_CARRIED_BLOCK = NULL;
static const EntityDataAccessor<bool>* DATA_CREEPY = NULL;

EnderMan::EnderMan(Level* level) : Monster(level), mLastCreepySound(0), mTargetChangeTime(0)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 2.9f);
    setPathfindingMalus(NULL, -1.0f);
}

EnderMan::~EnderMan()
{
}

void EnderMan::staticCtor()
{
    DATA_CARRIED_BLOCK = SynchedEntityData::defineId<const BlockState*>(0x5D540043ULL, ENTITY_TYPE_MOB, EntityDataSerializers::BLOCK_STATE);
    DATA_CREEPY = SynchedEntityData::defineId<bool>(0x5D540043ULL, ENTITY_TYPE_MOB, EntityDataSerializers::BOOLEAN);
}

void EnderMan::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(7, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mGoalSelector.addGoal(10, new EndermanLeaveBlockGoal(this), true);
    mGoalSelector.addGoal(11, new EndermanTakeBlockGoal(this), true);
    mTargetSelector.addGoal(1, new EndermanLookForPlayerGoal(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}

void EnderMan::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(40.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(7.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(64.0);
}

void EnderMan::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    if (target)
    {
        mTargetChangeTime = mTickCount;
        getEntityData()->set(DATA_CREEPY, true, true);
    }
    else
    {
        mTargetChangeTime = 0;
        getEntityData()->set(DATA_CREEPY, false, true);
    }
}

void EnderMan::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_CARRIED_BLOCK, (const BlockState*)NULL);
    getEntityData()->createDateItem(DATA_CREEPY, false);
}

void EnderMan::playCreepySound()
{
    if (mLastCreepySound + 400 <= mTickCount && !isSilent())
    {
        mLastCreepySound = mTickCount;
        playSound(SoundEvents::ENDERMEN_SCREAM, 2.5f, 1.0f);
    }
}

bool EnderMan::isCreepy()
{
    return getEntityData()->get(DATA_CREEPY);
}

void EnderMan::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor != NULL && isCreepy() && mLevel != NULL && mLevel->mIsClientSide)
        playCreepySound();
    LivingEntity::onSyncedDataUpdated(accessor);
}

void EnderMan::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

const BlockState* EnderMan::getCarryingBlock()
{
    return getEntityData()->get(DATA_CARRIED_BLOCK);
}

void EnderMan::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    const BlockState* state = getCarryingBlock();
    if (state != NULL)
    {
        tag->putShort(L"carried", (short)Block::getId(state->getBlock()));
        tag->putShort(L"carriedData", (short)Block::getIdWithData(state));
    }
}

void EnderMan::setCarryingBlock(const BlockState* state)
{
    getEntityData()->set(DATA_CARRIED_BLOCK, state, true);
}

void EnderMan::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    int id = tag->getShort(L"carried");
    const Block* block = Block::byId(id);
    const BlockState* state = NULL;
    if (block != NULL)
        state = Block::stateByIdWithData(tag->getShort(L"carriedData"));
    setCarryingBlock(state);
}

bool EnderMan::isLookingAtMe(boost::shared_ptr<Player> player)
{
    if (!player)
        return false;
    Vec3* view = player->getViewVector(1.0f);
    Vec3* direction = Vec3::newTemp(mX - player->mX, mY + (double)getEyeHeight() - player->mY - (double)player->getEyeHeight(), mZ - player->mZ);
    direction->normalize();
    return view->dot(direction) > 1.0 - 0.025 / direction->length() && player->canSee(shared_from_this());
}

double EnderMan::getEyeHeight()
{
    return 2.55;
}

void EnderMan::aiStep()
{
    if (mLevel->mIsClientSide)
        playCreepySound();
    Monster::aiStep();
}

bool EnderMan::teleport(double x, double y, double z)
{
    if (randomTeleport(x, y, z))
    {
        mLevel->levelEvent(boost::shared_ptr<Player>(), 1013, BlockPos((int)mX, (int)mY, (int)mZ), 0);
        return true;
    }
    return false;
}

bool EnderMan::teleport()
{
    return randomTeleport(mX + (mRandom->nextDouble() - 0.5) * 64.0,
                          mY + (mRandom->nextInt(64) - 32),
                          mZ + (mRandom->nextDouble() - 0.5) * 64.0);
}

void EnderMan::customServerAiStep()
{
    if (isInWaterRainOrBubble())
        teleport();
    if (mLevel->isDay() && mTickCount > mTargetChangeTime + 600 && getBrightness() > 0.5f && teleport())
        setTarget(boost::shared_ptr<LivingEntity>());
    Monster::customServerAiStep();
}

bool EnderMan::teleportTowards(boost::shared_ptr<Entity> entity)
{
    if (!entity)
        return false;
    return teleport(entity->mX + (mRandom->nextDouble() - 0.5) * 8.0,
                    entity->mY + mRandom->nextInt(8) - 4,
                    entity->mZ + (mRandom->nextDouble() - 0.5) * 8.0);
}

const SoundEvent* EnderMan::getAmbientSound() { return isCreepy() ? SoundEvents::ENDERMEN_SCREAM : SoundEvents::ENDERMEN_AMBIENT; }
const SoundEvent* EnderMan::getHurtSound(const DamageSource*) { return SoundEvents::ENDERMEN_HURT; }
const SoundEvent* EnderMan::getDeathSound() { return SoundEvents::ENDERMEN_DEATH; }
void EnderMan::dropEquipment(bool wasRecentlyHit, int lootingModifier)
{
    Monster::dropEquipment(wasRecentlyHit, lootingModifier);
    setCarryingBlock(NULL);
}
ResourceLocation* EnderMan::getDefaultLootTable() { return &BuiltInLootTables::ENDERMAN; }

bool EnderMan::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    if (!mLevel->mIsClientSide)
        teleport();
    return LivingEntity::hurt(source, damage);
}

void EnderMan::create(Level* level) { new EnderMan(level); }
unsigned long long EnderMan::GetType() { return 0x000000005D540043ULL; }

EnderMan::EndermanLeaveBlockGoal::EndermanLeaveBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan) { }
EnderMan::EndermanLeaveBlockGoal::~EndermanLeaveBlockGoal() { }
bool EnderMan::EndermanLeaveBlockGoal::canUse() { return mEnderMan->getCarryingBlock() != NULL && mEnderMan->mLevel->getGameRules()->getBoolean(1) && mEnderMan->mRandom->nextInt(2000) == 0; }
bool EnderMan::EndermanLeaveBlockGoal::canPlaceBlock(Level* level, const BlockPos& pos, Block* block,
                                                     const BlockState* oldState, const BlockState* newState)
{
    return block->mayPlace(level, pos)
        && oldState->getBlock() == Blocks::AIR
        && newState->getBlock() != Blocks::AIR && newState->isSolid();
}

void EnderMan::EndermanLeaveBlockGoal::tick()
{
    Random* random = mEnderMan->getRandom();
    double x = mEnderMan->mX - 1.0;
    double draw = random->nextDouble();
    x = x + draw;
    x = x + draw;
    double y = mEnderMan->mY;
    draw = random->nextDouble();
    y = y + draw;
    y = y + draw;
    double z = mEnderMan->mZ - 1.0;
    draw = random->nextDouble();
    z = z + draw;
    z = z + draw;
    BlockPos pos(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)),
                 static_cast<int>(std::floor(z)));
    BlockPos below = pos.below();
    const BlockState* state = mEnderMan->mLevel->getBlockState(pos);
    const BlockState* belowState = mEnderMan->mLevel->getBlockState(below);
    const BlockState* carried = mEnderMan->getCarryingBlock();
    if (carried != NULL && canPlaceBlock(mEnderMan->mLevel, pos, carried->getBlock(), state, belowState))
    {
        mEnderMan->mLevel->setBlock(pos, carried, 3, false);
        mEnderMan->setCarryingBlock(NULL);
    }
}
std::wstring EnderMan::EndermanLeaveBlockGoal::getName() { return L"EndermanLeaveBlock"; }

EnderMan::EndermanLookForPlayerGoal::EndermanLookForPlayerGoal(EnderMan* enderMan) : NearestAttackableTargetGoal(enderMan, typeid(Player), true, false, false), mTargetChangeTime(0), mAggression(0), mEnderMan(enderMan) { }
EnderMan::EndermanLookForPlayerGoal::~EndermanLookForPlayerGoal() { }
bool EnderMan::EndermanLookForPlayerGoal::canUse() { return NearestAttackableTargetGoal::canUse() && mEnderMan->isLookingAtMe(boost::static_pointer_cast<Player>(mTarget)); }
void EnderMan::EndermanLookForPlayerGoal::start() { mTargetChangeTime = 0; mAggression = 5; }
void EnderMan::EndermanLookForPlayerGoal::stop() { mTarget.reset(); mEnderMan->setTarget(boost::shared_ptr<LivingEntity>()); NearestAttackableTargetGoal::stop(); }
bool EnderMan::EndermanLookForPlayerGoal::canContinueToUse() { return mTarget && mEnderMan->isLookingAtMe(mTarget); }
void EnderMan::EndermanLookForPlayerGoal::tick() { if (mTarget) { boost::shared_ptr<Entity> entity(mTarget, (Entity*)mTarget.get()); mEnderMan->lookAt(entity, 30.0f, 30.0f); if (++mAggression > 600) mEnderMan->teleportTowards(entity); } }

EnderMan::EndermanTakeBlockGoal::EndermanTakeBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan) { }
EnderMan::EndermanTakeBlockGoal::~EndermanTakeBlockGoal() { }
bool EnderMan::EndermanTakeBlockGoal::canUse() { return mEnderMan->getCarryingBlock() == NULL && mEnderMan->mLevel->getGameRules()->getBoolean(1) && mEnderMan->mRandom->nextInt(20) == 0; }
void EnderMan::EndermanTakeBlockGoal::tick()
{
    if (mEnderMan->getCarryingBlock() == NULL && mEnderMan->mLevel != NULL)
    {
        BlockPos pos((int)mEnderMan->mX, (int)mEnderMan->mY - 1, (int)mEnderMan->mZ);
        const BlockState* state = mEnderMan->mLevel->getBlockState(pos);
        if (state != NULL && state->getBlock() != NULL)
            mEnderMan->setCarryingBlock(state);
    }
}
std::wstring EnderMan::EndermanTakeBlockGoal::getName() { return L"EndermanTakeBlock"; }
