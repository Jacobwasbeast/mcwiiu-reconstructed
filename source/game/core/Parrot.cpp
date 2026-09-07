#include "game/core/Parrot.hpp"

#include <cmath>

#include "game/core/DifficultyInstance.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

Parrot::Parrot(Level* level)
    : ShoulderRidingEntity(level), mFlap(0.0f), mFlapSpeed(0.0f), mOFlapSpeed(0.0f),
      mOFlap(0.0f), mFlapping(1.0f), mRecordPlayingNearby(false), mRecordPosition(NULL),
      mFlyingNavigation(NULL), mVariant(0)
{
    fjDerivedCtorCalls();
    _init();
    setSize(0.5f, 0.9f);
}

Parrot::~Parrot()
{
    delete mRecordPosition;
}

void Parrot::create(Level* level)
{
    new Parrot(level);
}

bool Parrot::isFlying()
{
    return !mOnGround;
}

const SoundEvent* Parrot::getImitatedSound(unsigned long long)
{
    return NULL;
}

double Parrot::getPitch(Random* random)
{
    return static_cast<double>((random->nextFloat() - random->nextFloat()) * 0.2f + 1.5f);
}

bool Parrot::imitateNearbyMobs(Level*, Entity*)
{
    return false;
}

const SoundEvent* Parrot::getAmbient(Random*)
{
    return NULL;
}

void Parrot::playAmbientSound(Level*, boost::shared_ptr<Entity>)
{
}

void Parrot::_init()
{
    mFlap = 0.0f;
    mFlapSpeed = 0.0f;
    mOFlapSpeed = 0.0f;
    mOFlap = 0.0f;
    mFlapping = 1.0f;
    mRecordPlayingNearby = false;
}

void Parrot::staticCtor()
{
}

void Parrot::setVariant(int variant)
{
    mVariant = variant < 0 ? 0 : (variant > 4 ? 4 : variant);
}

MobGroupData* Parrot::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& entityData)
{
    setVariant(mRandom == NULL ? 0 : mRandom->nextInt(5));
    return Mob::finalizeMobSpawn(difficulty, data, entityData);
}

void Parrot::registerGoals()
{
    Mob::registerGoals();
}

void Parrot::registerAttributes()
{
    Mob::registerAttributes();
}

FlyingPathNavigation* Parrot::createNavigation(Level* level)
{
    mFlyingNavigation = new FlyingPathNavigation(this, level);
    mFlyingNavigation->setCanOpenDoors(false);
    mFlyingNavigation->setCanFloat(true);
    mFlyingNavigation->setCanPassDoors(true);
    return mFlyingNavigation;
}

double Parrot::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.6f);
}

void Parrot::calculateFlapping()
{
    mOFlap = mFlap;
    mOFlapSpeed = mFlapSpeed;
    mFlapSpeed += (mOnGround ? -1.0f : 4.0f) * 0.3f;
    if (mFlapSpeed < 0.0f) mFlapSpeed = 0.0f;
    if (mFlapSpeed > 1.0f) mFlapSpeed = 1.0f;
    if (!mOnGround && mFlapping < 1.0f) mFlapping = 1.0f;
    mFlapping *= 0.9f;
    if (!mOnGround && mYd < 0.0) mYd *= 0.6;
    mFlap += mFlapping * 2.0f;
}

void Parrot::aiStep()
{
    Animal::aiStep();
    calculateFlapping();
}

void Parrot::setRecordPlayingNearby(const BlockPos& pos, bool playing)
{
    mRecordPlayingNearby = playing;
    if (playing)
    {
        delete mRecordPosition;
        mRecordPosition = new Vec3i(pos);
    }
}

bool Parrot::isPartyParrot()
{
    return mRecordPlayingNearby;
}

bool Parrot::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return TamableAnimal::interact(player, hand);
}

bool Parrot::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return isTameFood(itemInstance);
}

bool Parrot::checkSpawnRules()
{
    return Mob::checkSpawnRules();
}

void Parrot::causeFallDamage(float, float)
{
}

bool Parrot::isTameFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return itemInstance->getItem() != NULL;
}

unsigned int Parrot::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Animal::GetInteractTooltip(holder);
}

void Parrot::checkFallDamage(double, bool, Block*, const BlockPos&)
{
}

bool Parrot::canMate(const boost::shared_ptr<Animal>&)
{
    return false;
}

boost::shared_ptr<AgableMob> Parrot::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new Parrot(mLevel));
}

bool Parrot::doHurtTarget(boost::shared_ptr<Entity> target)
{
    return LivingEntity::doHurtTarget(target);
}

const SoundEvent* Parrot::getAmbientSound()
{
    return getAmbient(mRandom);
}

const SoundEvent* Parrot::getHurtSound(const DamageSource*)
{
    return NULL;
}

const SoundEvent* Parrot::getDeathSound()
{
    return NULL;
}

void Parrot::playStepSound(const BlockPos&, Block*)
{
}

float Parrot::playFlySound(float volume)
{
    return volume + mFlapSpeed * 0.5f;
}

bool Parrot::makeFlySound()
{
    return true;
}

float Parrot::getVoicePitch()
{
    return static_cast<float>(getPitch(mRandom));
}

SoundSource::ESoundSource Parrot::getSoundSource()
{
    return SoundSource::eSoundSource_Neutral;
}

bool Parrot::isPushable(boost::shared_ptr<Entity>)
{
    return true;
}

void Parrot::doPush(boost::shared_ptr<Entity> entity)
{
    LivingEntity::doPush(entity);
}

bool Parrot::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source)) return false;
    return Animal::hurt(source, damage);
}

int Parrot::getVariant()
{
    return mVariant;
}

void Parrot::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
}

void Parrot::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
}

void Parrot::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
}

const void* Parrot::getDefaultLootTable()
{
    return NULL;
}

unsigned long long Parrot::GetType()
{
    return 0x2d6c000300000000ULL;
}

bool Parrot::NotParrotPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && entity->GetType() != 0x2d6c000300000000ULL;
}

Parrot::NotParrotPredicate::~NotParrotPredicate()
{
}
