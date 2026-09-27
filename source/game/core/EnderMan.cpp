#include "game/core/EnderMan.hpp"
#include "game/core/IsLookingAtMePredicate.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Endermite.hpp"

#include <cmath>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
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
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/PIXNamedEvents.hpp"

static const EntityDataAccessor<const BlockState*>* DATA_CARRIED_BLOCK = NULL;
static const EntityDataAccessor<bool>* DATA_CREEPY = NULL;
static boost::unordered_set<Block*> sCarriableBlocks;

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
    DATA_CARRIED_BLOCK = SynchedEntityData::defineId<const BlockState*>(EntityType::ENDER_MAN, EntityType::MONSTER, EntityDataSerializers::BLOCK_STATE);
    DATA_CREEPY = SynchedEntityData::defineId<bool>(EntityType::ENDER_MAN, EntityType::MONSTER, EntityDataSerializers::BOOLEAN);
    sCarriableBlocks.insert(Blocks::grass);
    sCarriableBlocks.insert(Blocks::dirt);
    sCarriableBlocks.insert(Blocks::sand);
    sCarriableBlocks.insert(Blocks::gravel);
    sCarriableBlocks.insert(Blocks::yellow_flower);
    sCarriableBlocks.insert(Blocks::red_flower);
    sCarriableBlocks.insert(Blocks::brown_mushroom);
    sCarriableBlocks.insert(Blocks::red_mushroom);
    sCarriableBlocks.insert(Blocks::tnt);
    sCarriableBlocks.insert(Blocks::cactus);
    sCarriableBlocks.insert(Blocks::clay);
    sCarriableBlocks.insert(Blocks::pumpkin);
    sCarriableBlocks.insert(Blocks::melon_block);
    sCarriableBlocks.insert(Blocks::mycelium);
    sCarriableBlocks.insert(Blocks::netherrack);
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
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Endermite), true, false, false), true);
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
    if (mLastCreepySound + 400 <= mTickCount)
    {
        mLastCreepySound = mTickCount;
        if (!isSilent())
        {
            mLevel->playLocalSound(mX, mY + getEyeHeight(), mZ, SoundEvents::ENDERMEN_STARE, getSoundSource(), 2.5f,
                                   1.0f, false, 16.0f);
        }
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
    Mob::addDataWalkers(fixer, EntityType::ENDER_MAN);
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
    Block* block = tag->contains(L"carried", Tag::TAG_STRING)
        ? Block::byString(tag->getString(L"carried"))
        : Block::byId(tag->getShort(L"carried"));
    const BlockState* state = block->getBlockState(tag->getShort(L"carriedData"));
    if (state != NULL && state->getBlock() == NULL)
        state = NULL;
    setCarryingBlock(state);
}

bool EnderMan::isLookingAtMe(boost::shared_ptr<Player> player)
{
    if (player->mInventory->getArmor(3)->getItem() == Item::byBlock(Blocks::carved_pumpkin))
        return false;

    Vec3* view = player->getViewVector(1.0f)->normalize();
    Vec3* toEnderMan = Vec3::newTemp(mX - player->mX,
                                     mBoundingBox->mMinY + getEyeHeight() - (player->mY + player->getEyeHeight()),
                                     mZ - player->mZ);
    const double distance = toEnderMan->length();
    toEnderMan = toEnderMan->normalize();
    return view->dot(toEnderMan) > 1.0 - 0.025 / distance && player->canSee(shared_from_this());
}

double EnderMan::getEyeHeight()
{
    return 2.55;
}

void EnderMan::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        for (int i = 0; i < 2; ++i)
        {
            mLevel->addParticle(PARTICLE_ENDER, false, mX + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                mY + mRandom->nextDouble() * (double)mBbHeight - 0.25,
                                mZ + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                (mRandom->nextDouble() - 0.5) * 2.0, -mRandom->nextDouble(),
                                (mRandom->nextDouble() - 0.5) * 2.0, arrayWithLength<int>());
        }
    }
    mJumping = false;
    Monster::aiStep();
}

bool EnderMan::teleport(double x, double y, double z)
{
    const bool teleported = randomTeleport(x, y, z);
    if (teleported)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mXo, mYo, mZo, SoundEvents::ENDERMEN_TELEPORT, getSoundSource(),
                          1.0f, 1.0f, 16.0f);
        playSound(SoundEvents::ENDERMEN_TELEPORT, 1.0f, 1.0f);
    }
    return teleported;
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
        hurt(DamageSource::DROWN, 1.0f);
    if (mLevel->isDay() && mTickCount >= mTargetChangeTime + 600)
    {
        float brightness = (float)getBrightness();
        if (brightness > 0.5f)
        {
            BlockPos pos(shared_from_this());
            if (mLevel->canSeeSky(pos) && mRandom->nextFloat() * 30.0f < 2.0f * (brightness - 0.4f))
            {
                setTarget(boost::shared_ptr<LivingEntity>());
                teleport();
            }
        }
    }
    Mob::customServerAiStep();
}

bool EnderMan::teleportTowards(boost::shared_ptr<Entity> entity)
{
    Vec3* direction = Vec3::newTemp(mX - entity->mX,
                                    mBoundingBox->mMinY + mBbHeight / 2.0f - entity->mY + entity->getEyeHeight(),
                                    mZ - entity->mZ)->normalize();
    double x = mX + (mRandom->nextDouble() - 0.5) * 8.0 - direction->x * 16.0;
    double y = mY + (mRandom->nextInt(16) - 8) - direction->y * 16.0;
    double z = mZ + (mRandom->nextDouble() - 0.5) * 8.0 - direction->z * 16.0;
    return teleport(x, y, z);
}

const SoundEvent* EnderMan::getAmbientSound() { return isCreepy() ? SoundEvents::ENDERMEN_SCREAM : SoundEvents::ENDERMEN_AMBIENT; }
const SoundEvent* EnderMan::getHurtSound(const DamageSource*) { return SoundEvents::ENDERMEN_HURT; }
const SoundEvent* EnderMan::getDeathSound() { return SoundEvents::ENDERMEN_DEATH; }
void EnderMan::dropEquipment(bool wasRecentlyHit, int lootingModifier)
{
    Monster::dropEquipment(wasRecentlyHit, lootingModifier);
    const BlockState* state = getCarryingBlock();
    if (state != NULL)
    {
        Block* block = state->getBlock();
        Item* item = Item::byBlock(block);
        int data = item->isStackedByData() ? Block::getIdWithData(state) : 0;
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(item, 1, data)), 0.0f);
    }
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
unsigned long long EnderMan::GetType() { return EntityType::ENDER_MAN; }

EnderMan::EndermanLeaveBlockGoal::EndermanLeaveBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan) { }
EnderMan::EndermanLeaveBlockGoal::~EndermanLeaveBlockGoal() { }
bool EnderMan::EndermanLeaveBlockGoal::canUse() { return mEnderMan->getCarryingBlock() != NULL && mEnderMan->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) && mEnderMan->mRandom->nextInt(2000) == 0; }
bool EnderMan::EndermanLeaveBlockGoal::canPlaceBlock(Level* level, const BlockPos& pos, Block* block,
                                                     const BlockState* oldState, const BlockState* newState)
{
    return block->mayPlace(level, pos)
        && oldState->getMaterial() == Material::AIR
        && newState->getMaterial() != Material::AIR && newState->isCubeShaped();
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
        mEnderMan->mLevel->setBlock(pos, carried, Block::UPDATE_ALL, false);
        mEnderMan->setCarryingBlock(NULL);
    }
}
std::wstring EnderMan::EndermanLeaveBlockGoal::getName() { return L"EndermanLeaveBlock"; }

EnderMan::EndermanLookForPlayerGoal::EndermanLookForPlayerGoal(EnderMan* enderMan)
    : NearestAttackableTargetGoal(enderMan, typeid(Player), true, false, false), mAggroTime(0), mTeleportTime(0),
      mEnderMan(enderMan)
{
}

EnderMan::EndermanLookForPlayerGoal::~EndermanLookForPlayerGoal()
{
}

bool EnderMan::EndermanLookForPlayerGoal::canUse()
{
    const double range = getFollowDistance();
    IsLookingAtMePredicate predicate;
    predicate.mEnderMan = mEnderMan;
    mPendingTarget = boost::static_pointer_cast<Player>(mEnderMan->mLevel->getNearestAttackablePlayer(
        mEnderMan->mX, mEnderMan->mY, mEnderMan->mZ, range, range, NULL, &predicate));
    return mPendingTarget != NULL;
}

void EnderMan::EndermanLookForPlayerGoal::start()
{
    mAggroTime = 5;
    mTeleportTime = 0;
}

void EnderMan::EndermanLookForPlayerGoal::stop()
{
    mPendingTarget.reset();
    TargetGoal::stop();
}

bool EnderMan::EndermanLookForPlayerGoal::canContinueToUse()
{
    if (mPendingTarget == NULL)
        return NearestAttackableTargetGoal::canContinueToUse();
    if (!mEnderMan->isLookingAtMe(mPendingTarget))
        return false;
    mEnderMan->lookAt(boost::shared_ptr<Entity>(mPendingTarget), 10.0f, 10.0f);
    return true;
}

void EnderMan::EndermanLookForPlayerGoal::tick()
{
    if (mPendingTarget != NULL)
    {
        if (--mAggroTime > 0)
            return;
        NearestAttackableTargetGoal::mTarget = mPendingTarget;
        mPendingTarget.reset();
        NearestAttackableTargetGoal::start();
        return;
    }

    boost::shared_ptr<LivingEntity> target = NearestAttackableTargetGoal::mTarget.lock();
    if (target != NULL)
    {
        bool lookedAt = false;
        if ((target->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
            lookedAt = mEnderMan->isLookingAtMe(boost::static_pointer_cast<Player>(target));
        if (lookedAt)
        {
            if (target->distanceToSqr(mEnderMan->shared_from_this()) < 16.0)
                mEnderMan->teleport();
            mTeleportTime = 0;
        }
        else if (target->distanceToSqr(mEnderMan->shared_from_this()) > 256.0 && mTeleportTime++ >= 30 &&
                 mEnderMan->teleportTowards(target))
        {
            mTeleportTime = 0;
        }
    }
    Goal::tick();
}

EnderMan::EndermanTakeBlockGoal::EndermanTakeBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan) { }
EnderMan::EndermanTakeBlockGoal::~EndermanTakeBlockGoal() { }
bool EnderMan::EndermanTakeBlockGoal::canUse() { return mEnderMan->getCarryingBlock() == NULL && mEnderMan->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) && mEnderMan->mRandom->nextInt(20) == 0; }
void EnderMan::EndermanTakeBlockGoal::tick()
{
    Random* random = mEnderMan->getRandom();
    Level* level = mEnderMan->mLevel;
    int x = static_cast<int>(std::floor(mEnderMan->mX - 2.0 + random->nextDouble() * 4.0));
    int y = static_cast<int>(std::floor(mEnderMan->mY + random->nextDouble() * 3.0));
    int z = static_cast<int>(std::floor(mEnderMan->mZ - 2.0 + random->nextDouble() * 4.0));
    BlockPos pos(x, y, z);
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    Vec3* from = Vec3::newTemp(std::floor(mEnderMan->mX) + 0.5, (double)y + 0.5, std::floor(mEnderMan->mZ) + 0.5);
    Vec3* to = Vec3::newTemp((double)x + 0.5, (double)y + 0.5, (double)z + 0.5);
    HitResult* hit = level->clip(from, to, false, true, false, false, NULL, false);
    bool hitMatchesPos = hit != NULL && hit->getBlockPos().equals(pos);
    if (hitMatchesPos && sCarriableBlocks.find(block) != sCarriableBlocks.end())
    {
        mEnderMan->setCarryingBlock(state);
        level->removeBlock(pos);
    }
    if (hit != NULL)
        delete hit;
}
std::wstring EnderMan::EndermanTakeBlockGoal::getName() { return L"EndermanTakeBlock"; }
