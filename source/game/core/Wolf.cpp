#include "game/core/Wolf.hpp"

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/BegGoal.hpp"
#include "game/entity/ai/BabyOnLandSelector.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowOwnerGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/NonTameRandomTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtTargetGoal.hpp"
#include "game/entity/ai/PreySelector.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Llama.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<float>* Wolf::DATA_HEALTH = NULL;
const EntityDataAccessor<bool>* Wolf::DATA_INTERESTED = NULL;
const EntityDataAccessor<int>* Wolf::DATA_COLLAR_COLOR = NULL;
static Predicate<boost::shared_ptr<Entity> >* WOLF_PREY_SELECTOR = NULL;
static Predicate<boost::shared_ptr<Entity> >* WOLF_BABY_ON_LAND_SELECTOR = NULL;

Wolf::Wolf(Level* level)
    : TamableAnimal(level), mBodyRollAngle(0.0f), mInterestedAngle(0),
      mIsWet(0), mIsWetShaking(0), mHeadRollAngle(0.0f), mTailRollAngle(0.0f)
{
    setSize(0.6f, 0.85f);
}

Wolf::~Wolf() { }

void Wolf::create(Level* level) { new Wolf(level); }
void Wolf::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer, 0xd6c0002); }
void Wolf::staticCtor()
{
    DATA_HEALTH = SynchedEntityData::defineId<float>(0x000000000D6C0002ULL,
                                                     0x000000000D6C0000ULL,
                                                     EntityDataSerializers::FLOAT);
    DATA_INTERESTED = SynchedEntityData::defineId<bool>(0x000000000D6C0002ULL,
                                                        0x000000000D6C0000ULL,
                                                        EntityDataSerializers::BOOLEAN);
    DATA_COLLAR_COLOR = SynchedEntityData::defineId<int>(0x000000000D6C0002ULL,
                                                         0x000000000D6C0000ULL,
                                                         EntityDataSerializers::INT);
}
void Wolf::registerGoals()
{
    if (WOLF_PREY_SELECTOR == NULL)
        WOLF_PREY_SELECTOR = new PreySelector();
    if (WOLF_BABY_ON_LAND_SELECTOR == NULL)
        WOLF_BABY_ON_LAND_SELECTOR = new BabyOnLandSelector();
    if (mSitGoal != NULL)
    {
        delete mSitGoal;
        mSitGoal = NULL;
    }
    mSitGoal = new SitGoal(this);

    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, mSitGoal, false);
    mGoalSelector.addGoal(3, new WolfAvoidEntityGoal(this, typeid(Llama), 24.0f, 1.5, 1.5), true);
    mGoalSelector.addGoal(4, new LeapAtTargetGoal(this, 0.4f), true);
    mGoalSelector.addGoal(5, new MeleeAttackGoal(this, 1.0, true), true);
    mGoalSelector.addGoal(6, new FollowOwnerGoal(this, 1.0, 10.0f, 2.0f), true);
    mGoalSelector.addGoal(7, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(9, new BegGoal(this, 8.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(10, new RandomLookAroundGoal(this), true);

    mTargetSelector.addGoal(1, new OwnerHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new OwnerHurtTargetGoal(this), true);
    mTargetSelector.addGoal(3, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Animal), true, WOLF_PREY_SELECTOR, false), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Turtle), false, WOLF_BABY_ON_LAND_SELECTOR, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false), true);
}
void Wolf::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.30000001192092896);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(isTame() ? 20.0 : 8.0);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(2.0);
}
void Wolf::setAngry(bool angry)
{
    unsigned char flags = mEntityData->get(TamableAnimal::DATA_FLAGS);
    mEntityData->set(TamableAnimal::DATA_FLAGS, angry ? static_cast<unsigned char>(flags | 2)
                                                      : static_cast<unsigned char>(flags & 0xfd), true);
}
void Wolf::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    setAngry(target != NULL && !isTame());
}
void Wolf::customServerAiStep()
{
    mEntityData->set(DATA_HEALTH, getHealth(), true);
}
void Wolf::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    if (mEntityData != NULL && DATA_HEALTH != NULL) mEntityData->createDateItem(DATA_HEALTH, 1.0f);
    if (mEntityData != NULL && DATA_INTERESTED != NULL) mEntityData->createDateItem(DATA_INTERESTED, false);
    if (mEntityData != NULL && DATA_COLLAR_COLOR != NULL) mEntityData->createDateItem(DATA_COLLAR_COLOR, DyeColor::RED->getItemData());
}
void Wolf::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::WOLF_STEP, 0.15000000596046448f, 1.0f);
}
bool Wolf::isAngry() { return (mEntityData->get(TamableAnimal::DATA_FLAGS) & 2) != 0; }
const DyeColor* Wolf::getCollarColor() { return DyeColor::byItemData(mEntityData->get(DATA_COLLAR_COLOR) & 0xf); }
void Wolf::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Angry", isAngry());
    tag->putInt(L"CollarColor", getCollarColor()->getItemData());
}
void Wolf::setCollarColor(const DyeColor* color)
{
    mEntityData->set(DATA_COLLAR_COLOR, color->getItemData() & 0xf, true);
}
void Wolf::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setAngry(tag->getBoolean(L"Angry"));
    if (tag->contains(L"CollarColor")) setCollarColor(DyeColor::byItemData(tag->getInt(L"CollarColor")));
}
const SoundEvent* Wolf::getAmbientSound()
{
    if (isAngry()) return SoundEvents::WOLF_GROWL;
    if (getRandom()->nextInt(3) == 0)
    {
        if (isTame() && getHealth() < 10.0f) return SoundEvents::WOLF_WHINE;
        return SoundEvents::WOLF_PANT;
    }
    return SoundEvents::WOLF_AMBIENT;
}
const SoundEvent* Wolf::getHurtSound(const DamageSource*) { return SoundEvents::WOLF_HURT; }
const SoundEvent* Wolf::getDeathSound() { return SoundEvents::WOLF_DEATH; }
float Wolf::getSoundVolume() { return 0.4000000059604645f; }
const void* Wolf::getDefaultLootTable() { return &BuiltInLootTables::WOLF; }
void Wolf::aiStep()
{
    Animal::aiStep();
    if (mLevel != NULL && !mLevel->mIsClientSide && mIsWet && !mIsWetShaking && !isPathFinding() && mOnGround)
    {
        mHeadRollAngle = 0.0f;
        mTailRollAngle = 0.0f;
        mIsWetShaking = 1;
        mLevel->broadcastEntityEvent(shared_from_this(), 8, 0);
    }

    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        boost::shared_ptr<LivingEntity> target = getTarget();
        if (target == NULL || !target->isAlive())
        {
            if (isAngry())
                setAngry(false);
        }
    }
}
bool Wolf::isInterested() { return mEntityData->get(DATA_INTERESTED); }
bool Wolf::isWet() { return mIsWet != 0; }
void Wolf::tick()
{
    Mob::tick();
    mInterestedAngle = mBodyRollAngle;

    const float factor = 0.4000000059604645f;
    const float current = mBodyRollAngle;
    if (isInterested())
        mBodyRollAngle = (1.0f - current) * factor + current;
    else
        mBodyRollAngle = -current * factor + current;

    if (isInWaterRainOrBubble())
    {
        mHeadRollAngle = 0.0f;
        mIsWet = 1;
        mTailRollAngle = 0.0f;
        mIsWetShaking = 0;
    }
    else if (mIsWetShaking != 0)
    {
        const float oldHead = mHeadRollAngle;
        if (oldHead == 0.0f)
            playSound(SoundEvents::WOLF_SHAKE, getSoundVolume(),
                      (getRandom()->nextFloat() - getRandom()->nextFloat()) * 0.20000000298023224f + 1.0f);
        mHeadRollAngle = oldHead + 0.05000000074505806f;
        mTailRollAngle = oldHead;
        if (oldHead >= 2.0f)
        {
            mIsWet = 0;
            mIsWetShaking = 0;
            mTailRollAngle = 0.0f;
            mHeadRollAngle = 0.0f;
        }
        else
        {
            int count = static_cast<int>(Mth::sin((mHeadRollAngle - 0.4000000059604645f) * 3.1415927410125732f) * 7.0f);
            while (count-- > 0)
            {
                const double x = mX + (getRandom()->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                const double z = mZ + (getRandom()->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                mLevel->addParticle(PARTICLE_SPLASH, x, mY + 0.800000011920929f, z, mXd, mYd, mZd,
                                    arrayWithLength<int>());
            }
        }
    }

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target != NULL && Entity::instanceof(boost::static_pointer_cast<Entity>(target), 0x0C800000ULL)
        && mLevel->getDifficulty() == Difficulty::PEACEFUL)
        setTarget(boost::shared_ptr<LivingEntity>());
}
double Wolf::getBodyRollAngle(float partialTick, float scale)
{
    return mBodyRollAngle + (mHeadRollAngle - mBodyRollAngle) * partialTick * scale;
}
double Wolf::getHeadRollAngle(float partialTick) { return mHeadRollAngle * partialTick; }
double Wolf::getEyeHeight() { return mBbHeight * 0.8f; }
int Wolf::getMaxHeadXRot() { return 20; }
bool Wolf::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
bool Wolf::doHurtTarget(boost::shared_ptr<Entity> target) { return Mob::doHurtTarget(target); }
void Wolf::setTame(bool tame) { TamableAnimal::setTame(tame); }
void Wolf::tame(boost::shared_ptr<Player> player, bool success, bool showParticles)
{
    if (success) TamableAnimal::tame(player);
}
bool Wolf::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player)
        return false;
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty())
    {
        if (item->getItem() == Items::dye && isTame())
        {
            const DyeColor* color = DyeColor::byItemData(item->getAuxValue());
            if (color != NULL && color != getCollarColor())
            {
                setCollarColor(color);
                if (!player->mAbilities.mInstabuild)
                    item->shrink(1);
                return true;
            }
        }
        if (item->getItem() == Items::bone && !isTame() && !isAngry())
        {
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            tame(player, getRandom()->nextInt(3) == 0, true);
            return true;
        }
        if (AgeUpBaby(player, item))
            return true;
    }
    return Animal::mobInteract(player, hand);
}
void Wolf::handleEntityEvent(unsigned char event, int data)
{
    if (event == 8)
    {
        mIsWetShaking = 1;
        mHeadRollAngle = 0.0f;
        mTailRollAngle = 0.0f;
        return;
    }
    TamableAnimal::handleEntityEvent(event, data);
}
double Wolf::getTailAngle() { return mTailRollAngle; }
bool Wolf::isFood(const not_null_ptr<ItemInstance>& item) { return !item->isEmpty(); }
int Wolf::getMaxSpawnClusterSize() { return 4; }
int Wolf::GetSynchedHealth() { return static_cast<int>(getHealth()); }
unsigned int Wolf::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return Animal::GetInteractTooltip(tooltipData);
}
boost::shared_ptr<AgableMob> Wolf::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
void Wolf::setIsInterested(bool interested)
{
    mEntityData->set(DATA_INTERESTED, interested, true);
}
bool Wolf::canMate(const boost::shared_ptr<Animal>& other) { return Animal::canMate(other); }
bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target)
{
    return TamableAnimal::wantsToAttack(owner, target);
}
bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isAngry() && TamableAnimal::canBeLeashed(player); }
unsigned long long Wolf::GetType() { return 0x000000000D6C0002ULL; }
Wolf::WolfAvoidEntityGoal::WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType,
                                               float maxDistance, double walkSpeedModifier,
                                               double sprintSpeedModifier)
    : AvoidEntityGoal(wolf, avoidType, maxDistance, walkSpeedModifier, sprintSpeedModifier), mWolf(wolf) { }
Wolf::WolfAvoidEntityGoal::~WolfAvoidEntityGoal() { }
bool Wolf::WolfAvoidEntityGoal::avoidLlama(const boost::shared_ptr<Llama>& llama)
{
    return llama != NULL && mWolf != NULL && mWolf->getRandom()->nextInt(5) <= llama->getStrength();
}
bool Wolf::WolfAvoidEntityGoal::canUse() { return AvoidEntityGoal::canUse(); }
void Wolf::WolfAvoidEntityGoal::start() { AvoidEntityGoal::start(); }
void Wolf::WolfAvoidEntityGoal::tick() { AvoidEntityGoal::tick(); }
