#include "game/entity/mob/AbstractHorse.hpp"

#include <typeinfo>

#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/HorseChest.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"

const EntityDataAccessor<unsigned char>* AbstractHorse::DATA_FLAGS = NULL;
const EntityDataAccessor<std::wstring>* AbstractHorse::DATA_OWNER_UUID = NULL;

AbstractHorse::AbstractHorse(Level* level)
    : Animal(level), mFlags(0), mJumping(false), mStanding(false), mEating(false), mBred(false), mSaddled(false), mTamed(false), mTemper(0), mTailCounter(0), mJumpRidingScale(0.0f), mOwnerUUID(), mInventory(), mPanicGoal(0), mFloatGoal(0), mWaterAvoidingRandomStrollGoal(0)
{
}

AbstractHorse::~AbstractHorse()
{
}

bool AbstractHorse::getFlag(int flag) { return (mEntityData->get(DATA_FLAGS) & flag) != 0; }
bool AbstractHorse::isSaddled() { return getFlag(4); }
void AbstractHorse::containerChanged(Container*)
{
    const bool saddled = isSaddled();
    updateEquipment();
    if (!saddled && isSaddled() && mTickCount > 20)
        playSound(SoundEvents::HORSE_SADDLE, 0.5f, 1.0f);
}
void AbstractHorse::setFlag(int flag, bool value)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    mEntityData->set(DATA_FLAGS, value ? static_cast<unsigned char>(flags | flag) : static_cast<unsigned char>(flags & ~flag), true);
}
void AbstractHorse::setEating(bool eating) { mEating = eating; }
void AbstractHorse::setStanding(bool standing) { mStanding = standing; }
void AbstractHorse::stand() { setStanding(true); }
void AbstractHorse::onPlayerJump(int jump)
{
    if (!isSaddled() && GetType() != 0x2D690002ULL)
        return;
    if (jump < 0)
    {
        setJumpRidingScale(0.4f);
        return;
    }
    setEating(false);
    stand();
    if (jump > 89)
    {
        setJumpRidingScale(1.0f);
        return;
    }
    setJumpRidingScale(0.4f + (0.4f * (float)jump / 90.0f));
}
bool AbstractHorse::canJump() { return isSaddled() || GetType() == 0x2d690002ULL; }
double AbstractHorse::getJumpRidingScale() { return mJumpRidingScale; }
void AbstractHorse::setJumpRidingScale(float scale) { mJumpRidingScale = scale; }
void AbstractHorse::handleStartJump(int) { setIsJumping(true); }
void AbstractHorse::handleStopJump() { setIsJumping(false); }
std::wstring AbstractHorse::getOwnerUUID() { return mEntityData->get(DATA_OWNER_UUID); }
boost::shared_ptr<LivingEntity> AbstractHorse::getOwner() { return boost::shared_ptr<LivingEntity>(); }
int AbstractHorse::getTailCounter() const { return mTailCounter; }
void AbstractHorse::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId(0, 0x000000000D690000ULL, EntityDataSerializers::BYTE);
    DATA_OWNER_UUID = SynchedEntityData::defineId(0, 0x000000000D690000ULL, EntityDataSerializers::STRING);
}
void AbstractHorse::fjDerivedCtorCalls()
{
    Mob::fjDerivedCtorCalls();
    createBodyControl();
    updateEquipment();
}
void AbstractHorse::registerGoals()
{
    mFloatGoal = new FloatGoal(this);
    mGoalSelector.addGoal(0, mFloatGoal, true);

    mPanicGoal = new PanicGoal(this, 1.2);
    mGoalSelector.addGoal(1, mPanicGoal, true);
    mGoalSelector.addGoal(1, new RunAroundLikeCrazyGoal(this, 1.2), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0, typeid(AbstractHorse)), true);
    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.0), true);

    mWaterAvoidingRandomStrollGoal = new WaterAvoidingRandomStrollGoal(this, 0.7);
    mGoalSelector.addGoal(6, mWaterAvoidingRandomStrollGoal, true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}
void AbstractHorse::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
    mEntityData->createDateItem(DATA_OWNER_UUID, std::wstring(L""));
}
bool AbstractHorse::isTamed() { return getFlag(2); }
void AbstractHorse::setOwnerUUID(const std::wstring& uuid) { mEntityData->set(DATA_OWNER_UUID, uuid, true); }
double AbstractHorse::getFoalScale() const { return 1.0; }
void AbstractHorse::updateSize(bool baby) { if (baby) internalSetSize(0.8f); else internalSetSize(1.0f); }
bool AbstractHorse::isJumping() const { return mJumping; }
void AbstractHorse::setTamed(bool tamed) { setFlag(2, tamed); }
void AbstractHorse::setIsJumping(bool jumping) { mJumping = jumping; }
bool AbstractHorse::canBeLeashed(const boost::shared_ptr<Player>&) { return true; }
bool AbstractHorse::isEating() { return mEating; }
void AbstractHorse::onLeashDistance(float distance)
{
    if (distance > 12.0f && isEating())
        setEating(false);
}
bool AbstractHorse::isStanding() { return mStanding; }
bool AbstractHorse::isBred() { return mBred; }
void AbstractHorse::setBred(bool bred) { mBred = bred; }
void AbstractHorse::setSaddled(bool saddled) { setFlag(4, saddled); }
int AbstractHorse::getTemper() const { return mTemper; }
void AbstractHorse::setTemper(int temper) { mTemper = temper; }
int AbstractHorse::modifyTemper(int amount) { mTemper += amount; return mTemper; }
bool AbstractHorse::hurt(const DamageSource*, float) { return true; }
bool AbstractHorse::isPushable(boost::shared_ptr<Entity> entity) { return !isVehicle() || (entity && entity->GetType() == 0x08000008ULL); }
void AbstractHorse::openMouth() { setEating(true); }
void AbstractHorse::eating(int) { setEating(true); }
void AbstractHorse::causeFallDamage(float, float) { }
bool AbstractHorse::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand) { return false; }
int AbstractHorse::getInventorySize() { return 2; }
void AbstractHorse::createInventory()
{
    mInventory = boost::shared_ptr<Container>(new HorseChest(L"HorseChest", getInventorySize()));
}
void AbstractHorse::updateEquipment()
{
    if (mLevel == NULL || mRemoved)
        return;
    bool saddled = false;
    if (mInventory)
    {
        not_null_ptr<ItemInstance> saddle = mInventory->getItem(0);
        saddled = !saddle->isEmpty() && canBeSaddled();
    }
    setSaddled(saddled);
}
boost::shared_ptr<Entity> AbstractHorse::getClosestMommy(const boost::shared_ptr<Entity>&, double) { return boost::shared_ptr<Entity>(); }
double AbstractHorse::getCustomJump()
{
    return getAttributes()->getInstance(eATTRIBUTE_ID_HorseJumpStrength)->getValue();
}
const SoundEvent* AbstractHorse::getJumpSound() { return SoundEvents::HORSE_JUMP; }
const SoundEvent* AbstractHorse::getDeathSound() { return 0; }
const SoundEvent* AbstractHorse::getHurtSound(const DamageSource*) { return 0; }
const SoundEvent* AbstractHorse::getAmbientSound() { return 0; }
const SoundEvent* AbstractHorse::getAngrySound() { return 0; }
bool AbstractHorse::canBeSaddled() const { return true; }
void AbstractHorse::playStepSound(const BlockPos&, Block*) { }
void AbstractHorse::playGallopSound(const SoundType* sound)
{
    if (sound != NULL)
        playSound(sound->getStepSound(), sound->getVolume() * 0.4f, sound->getPitch());
}
void AbstractHorse::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->getInstance(eATTRIBUTE_ID_HorseJumpStrength)->setBaseValue(generateRandomJumpStrength());
}
int AbstractHorse::getMaxSpawnClusterSize() { return 6; }
int AbstractHorse::getMaxTemper() const { return 100; }
float AbstractHorse::getSoundVolume() { return 1.0f; }
int AbstractHorse::getAmbientSoundInterval() { return 400; }
void AbstractHorse::openInventory(const boost::shared_ptr<Player>&) { }
int AbstractHorse::CheckAndSetInLove(boost::shared_ptr<Player>) { return 0; }
int AbstractHorse::handleEating(const boost::shared_ptr<Player>&, const not_null_ptr<ItemInstance>&) { return 0; }
void AbstractHorse::doPlayerRide(const boost::shared_ptr<Player>& player)
{
    player->mYRot = mYRot;
    player->mXRot = mXRot;
    setEating(false);
    setStanding(false);
    if (mLevel->mIsClientSide)
        return;

    boost::shared_ptr<Entity> self = shared_from_this();
    player->startRiding(self);
}
bool AbstractHorse::isImmobile() { return LivingEntity::isImmobile() || (isVehicle() && isSaddled()) || isEating() || isStanding(); }
bool AbstractHorse::isFood(const not_null_ptr<ItemInstance>&) { return false; }
void AbstractHorse::moveTail() { mTailCounter = 1; }
void AbstractHorse::die(const DamageSource*) { }
void AbstractHorse::aiStep() { }
boost::shared_ptr<Entity> AbstractHorse::followMommy() { return boost::shared_ptr<Entity>(); }
bool AbstractHorse::canEatGrass() const { return true; }
void AbstractHorse::tick() { }
boost::shared_ptr<Entity> AbstractHorse::makeMad() { return boost::shared_ptr<Entity>(); }
bool AbstractHorse::tameWithName(const boost::shared_ptr<Player>&) { return false; }
void AbstractHorse::travel(float x, float y, float z)
{
    boost::shared_ptr<Entity> passenger = getControllingPassenger();
    if (!canBeControlledByRider() || passenger == 0)
    {
        setSpeed(0.02f);
        LivingEntity::travel(x, y, z);
        return;
    }

    setRot(passenger->mYRot, passenger->mXRot * 0.5f);
    mYRotO = mYRot;
    mXRotO = mXRot;
    setSpeed(static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue() * 0.1));

    boost::shared_ptr<LivingEntity> rider = boost::static_pointer_cast<LivingEntity>(passenger);
    float forward = rider->mZza;
    float strafe = rider->mXxa * 0.5f;
    if (isStanding() && !mJumping)
    {
        forward = 0.0f;
        strafe = 0.0f;
    }
    if (mJumpRidingScale > 0.0f && !mJumping)
    {
        mYd = getCustomJump() * mJumpRidingScale;
        if (hasEffect(MobEffects::JUMP))
            mYd += static_cast<float>(getEffect(MobEffects::JUMP)->getAmplifier() + 1) * 0.1f;
        mJumping = true;
        if (forward > 0.0f)
        {
            const float yaw = mYRot * 0.01745329238474369f;
            mXd += -Mth::sin(yaw) * mJumpRidingScale * 0.4f;
            mZd += Mth::cos(yaw) * mJumpRidingScale * 0.4f;
        }
        mJumpRidingScale = 0.0f;
    }

    if (isVehicle())
        LivingEntity::travel(strafe, y, forward);
    else
        LivingEntity::travel(x, y, z);
    if (isVehicle())
    {
        mJumpRidingScale = 0.0f;
        if (mJumping)
            setStanding(false);
        setIsJumping(false);
    }
    updateModelAnimation();
}
void AbstractHorse::updateModelAnimation() { }
void AbstractHorse::addDataWalkers(DataFixerUpper* fixer, unsigned long long type)
{
    Mob::addDataWalkers(fixer, type);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, new ItemDataWalker(type, L"SaddleItem"));
}
void AbstractHorse::addAdditonalSaveData(CompoundTag*) { }
void AbstractHorse::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag == NULL)
        return;
    Animal::readAdditionalSaveData(tag);
    setEating(tag->getBoolean(L"EatingHaystack"));
    setBred(tag->getBoolean(L"Bred"));
    setTemper(tag->getInt(L"Temper"));
    setTamed(tag->getBoolean(L"Tame"));
    std::wstring owner;
    if (tag->contains(L"OwnerUUID", 8))
        owner = tag->getString(L"OwnerUUID");
    else if (tag->contains(L"OwnerName", 8))
        owner = tag->getString(L"OwnerName");
    if (!owner.empty())
        setOwnerUUID(owner);
    if (!mInventory)
        createInventory();
    if (tag->contains(L"SaddleItem", 10))
    {
        ItemInstance saddle(tag->getCompound(L"SaddleItem"));
        if (!saddle.isEmpty() && saddle.getItem() == Items::saddle)
            mInventory->setItem(0, not_null_ptr<ItemInstance>(new ItemInstance(saddle)));
    }
    else if (tag->getBoolean(L"Saddle"))
    {
        mInventory->setItem(0, not_null_ptr<ItemInstance>(new ItemInstance(Items::saddle)));
    }
}
bool AbstractHorse::canMate(const boost::shared_ptr<Animal>&) { return false; }
bool AbstractHorse::canParent() { return !isVehicle() && isTamed() && !isBaby() && getAge() >= 0 && isInLove(); }
boost::shared_ptr<AgableMob> AbstractHorse::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
double AbstractHorse::generateRandomMaxHealth() { return 15.0; }
double AbstractHorse::generateRandomJumpStrength() { return 0.4; }
double AbstractHorse::generateRandomSpeed() { return 0.225; }
void AbstractHorse::setOffspringAttributes(const boost::shared_ptr<AgableMob>&, const boost::shared_ptr<AbstractHorse>&) { }
bool AbstractHorse::canBeControlledByRider() { return isSaddled() && isVehicle() && getControllingPassenger() != boost::shared_ptr<Entity>(); }
double AbstractHorse::getEatAnim(float) const { return 0.0; }
double AbstractHorse::getStandAnim(float) const { return 0.0; }
double AbstractHorse::getMouthAnim(float) const { return 0.0; }
void AbstractHorse::spawnTamingParticles(bool success)
{
    const ParticleType* particle = success ? PARTICLE_HEART : PARTICLE_SMOKE;
    const double velocityScale = 0.02;
    const double half = 0.5;
    for (int i = 0; i < 7; ++i)
    {
        const double gaussianX = mRandom->nextGaussian();
        const double gaussianY = mRandom->nextGaussian();
        const double gaussianZ = mRandom->nextGaussian();
        const float offsetX = static_cast<float>(mRandom->nextFloat() * mBbWidth);
        const float offsetY = static_cast<float>(mRandom->nextFloat() * mBbHeight);
        const float offsetZ = static_cast<float>(mRandom->nextFloat() * mBbWidth);
        const float doubledX = offsetX + offsetX;
        const float doubledZ = offsetZ + offsetZ;
        mLevel->addParticle(particle,
                            mX + static_cast<double>(doubledX) - mBbWidth,
                            mY + half + static_cast<double>(offsetY),
                            mZ + static_cast<double>(doubledZ) - mBbWidth,
                            gaussianX * velocityScale,
                            gaussianY * velocityScale,
                            gaussianZ * velocityScale,
                            arrayWithLength<int>());
    }
}
void AbstractHorse::handleEntityEvent(unsigned char event, int)
{
    if (event == 6)
        spawnTamingParticles(false);
    else if (event == 7)
        spawnTamingParticles(true);
    else
        Mob::handleEntityEvent(event, 0);
}
void AbstractHorse::positionRider(const boost::shared_ptr<Entity>&) { }
bool AbstractHorse::onLadder() { return false; }
double AbstractHorse::getEyeHeight() { return 1.0; }
bool AbstractHorse::wearsArmor() { return false; }
bool AbstractHorse::isArmor(const not_null_ptr<ItemInstance>&) { return false; }
bool AbstractHorse::setSlot(int, const not_null_ptr<ItemInstance>&) { return false; }
boost::shared_ptr<Entity> AbstractHorse::getControllingPassenger() { return boost::shared_ptr<Entity>(); }
MobGroupData* AbstractHorse::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&) { return data; }
bool AbstractHorse::removeWhenFarAway() { return Animal::removeWhenFarAway() && !isTamed() && !isSaddled() && !isVehicle(); }
unsigned long long AbstractHorse::GetType() { return 0x000000000D690000ULL; }
