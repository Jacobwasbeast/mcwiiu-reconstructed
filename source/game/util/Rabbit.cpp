#include "game/util/Rabbit.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

Rabbit::EvilRabbitAttackGoal::EvilRabbitAttackGoal(Rabbit* rabbit)
    : MeleeAttackGoal(rabbit, 1.4, true)
{
}

Rabbit::EvilRabbitAttackGoal::~EvilRabbitAttackGoal()
{
}

double Rabbit::EvilRabbitAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return static_cast<double>(target->mBbWidth * target->mBbWidth) + 4.0;
}

Rabbit::RabbitAvoidEntityGoal::RabbitAvoidEntityGoal(Rabbit* rabbit, const std::type_info& avoidType,
                                                     float maxDistance, double walkSpeedModifier,
                                                     double sprintSpeedModifier)
    : AvoidEntityGoal(rabbit, avoidType, maxDistance, walkSpeedModifier, sprintSpeedModifier),
      mRabbit(rabbit)
{
}

Rabbit::RabbitAvoidEntityGoal::~RabbitAvoidEntityGoal()
{
}

bool Rabbit::RabbitAvoidEntityGoal::canUse()
{
    return mRabbit->getRabbitType() != 99 && AvoidEntityGoal::canUse();
}

Rabbit::RabbitGroupData::RabbitGroupData(int type)
    : mRabbitType(type)
{
}

Rabbit::RabbitGroupData::~RabbitGroupData()
{
}

Rabbit::RabbitJumpControl::RabbitJumpControl(Rabbit* rabbit)
    : JumpControl(rabbit), mRabbit(rabbit), mCanJump(false)
{
}

Rabbit::RabbitJumpControl::~RabbitJumpControl()
{
}

void Rabbit::RabbitJumpControl::setCanJump(bool canJump)
{
    mCanJump = canJump;
}

unsigned char Rabbit::RabbitJumpControl::wantJump()
{
    return mJump ? 1 : 0;
}

unsigned char Rabbit::RabbitJumpControl::canJump()
{
    return mCanJump ? 1 : 0;
}

void Rabbit::RabbitJumpControl::tick()
{
    if (mJump)
    {
        mRabbit->startJumping();
        mJump = false;
    }
}

Rabbit::RabbitMoveControl::RabbitMoveControl(Rabbit* rabbit)
    : MoveControl(rabbit), mRabbit(rabbit), mSpeedModifier(0.0)
{
}

Rabbit::RabbitMoveControl::~RabbitMoveControl()
{
}

void Rabbit::RabbitMoveControl::tick()
{
    MoveControl::tick();
}

void Rabbit::RabbitMoveControl::setWantedPosition(double x, double y, double z, double speedModifier)
{
    if (mRabbit->isInWater()) speedModifier = 1.5;
    MoveControl::setWantedPosition(x, y, z, speedModifier);
    if (speedModifier > 0.0) mSpeedModifier = speedModifier;
}

Rabbit::RabbitPanicGoal::RabbitPanicGoal(Rabbit* rabbit, double speedModifier)
    : PanicGoal(rabbit, speedModifier), mRabbit(rabbit)
{
}

Rabbit::RabbitPanicGoal::~RabbitPanicGoal()
{
}

void Rabbit::RabbitPanicGoal::tick()
{
    PanicGoal::tick();
    if (mRabbit->mLevel != NULL) mRabbit->setSpeedModifier(mSpeedModifier);
}

Rabbit::RaidGardenGoal::RaidGardenGoal(Rabbit* rabbit)
    : MoveToBlockGoal(rabbit, 0.7, 16, 1, 1.0), mRabbit(rabbit),
      mHasTarget(false), mReachedTarget(false)
{
}

Rabbit::RaidGardenGoal::~RaidGardenGoal()
{
}

bool Rabbit::RaidGardenGoal::canUse()
{
    if (mNextStartTick < 1)
    {
        if (mRabbit->mLevel == NULL || !mRabbit->mLevel->getGameRules()->getBoolean(1)) return false;
        mHasTarget = true;
        mReachedTarget = false;
    }
    return mHasTarget && MoveToBlockGoal::canUse();
}

bool Rabbit::RaidGardenGoal::canContinueToUse()
{
    return mReachedTarget && MoveToBlockGoal::canContinueToUse();
}

void Rabbit::RaidGardenGoal::tick()
{
    MoveToBlockGoal::tick();
    if (isReachedTarget())
    {
        mReachedTarget = true;
        mRabbit->eatCarrot();
    }
}

bool Rabbit::RaidGardenGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    return level != NULL && level->getBlockState(pos)->getBlock() == Blocks::carrots;
}

Rabbit::Rabbit(Level* level)
    : Animal(level), mJumpTicks(0), mJumpDuration(0), mJumping(false),
      mLandingDelay(0), mMoreCarrotTicks(0), mUnknown(NULL), mRabbitType(0),
      mSnowType(0), mDesertType(0), mMountainType(0)
{
    fjDerivedCtorCalls();
    setSize(0.4f, 0.5f);
    setSpeedModifier(0.0);
}

Rabbit::~Rabbit()
{
}

void Rabbit::create(Level* level)
{
    new Rabbit(level);
}

void Rabbit::staticCtor()
{
    static const EntityDataAccessor<int>* rabbitType = NULL;
    rabbitType = SynchedEntityData::defineId<int>(0x2d680005ULL, 0x0d680000ULL,
                                                   EntityDataSerializers::INT);
    (void)rabbitType;
}

void Rabbit::setSpeedModifier(double speedModifier)
{
    MoveControl* control = static_cast<MoveControl*>(getMoveControl());
    if (control != NULL) control->setSpeedModifier(speedModifier);
}

void Rabbit::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new RabbitPanicGoal(this, 1.2), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 0.8), true);

    arrayWithLength<Item*> carrotItems(1, false);
    carrotItems.data[0] = Items::carrot;
    mGoalSelector.addGoal(3, new TemptGoal(this, 1.0, carrotItems, false,
                                            arrayWithLength<int>()), true);
    mGoalSelector.addGoal(5, new RaidGardenGoal(this), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(11, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
}

float Rabbit::getJumpPower()
{
    return 0.5;
}

void Rabbit::jumpFromGround()
{
    LivingEntity::jumpFromGround();
    if (getMoveControl() != NULL && static_cast<MoveControl*>(getMoveControl())->getSpeedModifier() > 0.0)
        moveRelative(0.0f, 0.0f, 0.1f, 0.0f);
}

double Rabbit::getJumpCompletion(float partialTick)
{
    if (mJumpDuration == 0) return 0.0;
    return static_cast<double>(mJumpTicks + partialTick) / static_cast<double>(mJumpDuration);
}

const SoundEvent* Rabbit::getJumpSound()
{
    return SoundEvents::RABBIT_JUMP;
}

void Rabbit::setJumping(bool jumping)
{
    LivingEntity::setJumping(jumping);
    mJumping = jumping;
    if (jumping) playSound(getJumpSound(), 0.3f, 1.0f);
}

void Rabbit::startJumping()
{
    setJumping(true);
    mJumpDuration = 10;
    mJumpTicks = 0;
}

bool Rabbit::checkSpawnRules()
{
    return PathfinderMob::checkSpawnRules();
}

void Rabbit::defineSynchedData()
{
    Animal::defineSynchedData();
}

void Rabbit::setLandingDelay()
{
    mLandingDelay = getMoveControl() != NULL && static_cast<MoveControl*>(getMoveControl())->getSpeedModifier() >= 2.2 ? 1 : 10;
}

void Rabbit::disableJumpControl()
{
    RabbitJumpControl* control = static_cast<RabbitJumpControl*>(getJumpControl());
    if (control != NULL) control->setCanJump(false);
}

void Rabbit::checkLandingDelay()
{
    setLandingDelay();
    disableJumpControl();
}

int Rabbit::getRabbitType()
{
    return mRabbitType;
}

void Rabbit::facePoint(double x, double z)
{
    mYRot = static_cast<float>(std::atan2(z - mZ, x - mX) * 57.29577951308232 - 90.0);
}

void Rabbit::enableJumpControl()
{
    RabbitJumpControl* control = static_cast<RabbitJumpControl*>(getJumpControl());
    if (control != NULL) control->setCanJump(true);
}

void Rabbit::customServerAiStep()
{
    Animal::customServerAiStep();
}

void Rabbit::updateSprintingState()
{
}

void Rabbit::aiStep()
{
    Animal::aiStep();
    if (mJumpTicks != mJumpDuration) ++mJumpTicks;
    else if (mJumpDuration != 0)
    {
        mJumpTicks = 0;
        mJumpDuration = 0;
        setJumping(false);
    }
}

void Rabbit::registerAttributes()
{
    Mob::registerAttributes();
}

void Rabbit::addDataWalkers(DataFixerUpper*)
{
}

void Rabbit::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putInt(L"RabbitType", getRabbitType());
    tag->putInt(L"MoreCarrotTicks", mMoreCarrotTicks);
}

void Rabbit::setRabbitType(int type)
{
    mRabbitType = type;
}

void Rabbit::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setRabbitType(tag->getInt(L"RabbitType"));
    mMoreCarrotTicks = tag->getInt(L"MoreCarrotTicks");
}

const SoundEvent* Rabbit::getAmbientSound()
{
    return SoundEvents::RABBIT_AMBIENT;
}

const SoundEvent* Rabbit::getHurtSound(const DamageSource*)
{
    return SoundEvents::RABBIT_HURT;
}

const SoundEvent* Rabbit::getDeathSound()
{
    return SoundEvents::RABBIT_DEATH;
}

bool Rabbit::doHurtTarget(boost::shared_ptr<Entity> target)
{
    return LivingEntity::doHurtTarget(target);
}

SoundSource::ESoundSource Rabbit::getSoundSource()
{
    return mRabbitType == 99 ? SoundSource::eSoundSource_Hostile : SoundSource::eSoundSource_Neutral;
}

bool Rabbit::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source)) return false;
    return Animal::hurt(source, damage);
}

ResourceLocation* Rabbit::getDefaultLootTable()
{
    return &BuiltInLootTables::RABBIT;
}

bool Rabbit::isTemptingItem(Item* item)
{
    return item == Items::carrot || item == Items::goldenCarrot || item == Item::byBlock(Blocks::carrots);
}

int Rabbit::getRandomRabbitType()
{
    int value = mRandom->nextInt(100);
    if (value < 5) return 4;
    if (value < 50) return 0;
    if (value < 90) return 2;
    return 5;
}

boost::shared_ptr<AgableMob> Rabbit::getBreedOffspring(boost::shared_ptr<AgableMob> parent)
{
    boost::shared_ptr<Rabbit> child(new Rabbit(mLevel));
    Rabbit* other = dynamic_cast<Rabbit*>(parent.get());
    child->setRabbitType(mRandom->nextInt(20) == 0 && other != NULL ? other->getRabbitType() : getRandomRabbitType());
    return child;
}

bool Rabbit::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return isTemptingItem(itemInstance->getItem());
}

MobGroupData* Rabbit::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                      const std::wstring& entityData)
{
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, data, entityData);
    RabbitGroupData* rabbitData = dynamic_cast<RabbitGroupData*>(result);
    if (rabbitData == NULL)
    {
        int type = getRandomRabbitType();
        setRabbitType(type);
        return new RabbitGroupData(type);
    }
    setRabbitType(rabbitData->mRabbitType);
    return result;
}

void Rabbit::eatCarrot()
{
    mMoreCarrotTicks = 40;
}

void Rabbit::handleEntityEvent(unsigned char event, int data)
{
    Animal::handleEntityEvent(event, data);
}

unsigned long long Rabbit::GetType()
{
    return 0x2d680005ULL;
}
