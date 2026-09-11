#include "game/core/Parrot.hpp"

#include <cmath>
#include <iterator>
#include <boost/unordered_map.hpp>

#include "game/core/DifficultyInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/world/Level.hpp"

namespace
{
boost::unordered_map<unsigned long long, const SoundEvent*> gImitatedSounds;
}
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowMobGoal.hpp"
#include "game/entity/ai/FollowOwnerFlyingGoal.hpp"
#include "game/entity/ai/LandOnOwnersShoulderGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomFlyingGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

const EntityDataAccessor<int>* Parrot::DATA_VARIANT = NULL;

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

const SoundEvent* Parrot::getImitatedSound(unsigned long long type)
{
    boost::unordered_map<unsigned long long, const SoundEvent*>::const_iterator it =
        gImitatedSounds.find(type);
    return it == gImitatedSounds.end() ? NULL : it->second;
}

double Parrot::getPitch(Random* random)
{
    return static_cast<double>((random->nextFloat() - random->nextFloat()) * 0.2f + 1.5f);
}

bool Parrot::imitateNearbyMobs(Level* level, Entity* entity)
{
    if (level == NULL || entity == NULL || entity->isSilent())
        return false;
    Random* random = level->getRandomFor(0, 0, 0);
    if (random == NULL || random->nextInt(50) != 0)
        return false;
    AABB* box = entity->getBoundingBoxForCulling();
    if (box == NULL)
        return false;
    AABB* area = box->grow(20.0);
    NotParrotPredicate predicate;
    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntitiesOfClass(typeid(Mob), area, &predicate, false);
    if (entities == NULL || entities->empty())
    {
        delete area;
        delete entities;
        return false;
    }
    boost::shared_ptr<Entity> selected = (*entities)[random->nextInt((int)entities->size())];
    const SoundEvent* sound = selected ? getImitatedSound(selected->GetType()) : NULL;
    if (selected && !selected->isSilent() && sound != NULL)
        selected->playSound(sound, 0.7f, (float)getPitch(random));
    delete area;
    delete entities;
    return selected && !selected->isSilent() && sound != NULL;
}

const SoundEvent* Parrot::getAmbient(Random* random)
{
    if (random == NULL || random->nextInt(1000) != 0 || gImitatedSounds.empty())
        return NULL;
    boost::unordered_map<unsigned long long, const SoundEvent*>::const_iterator it =
        gImitatedSounds.begin();
    std::advance(it, random->nextInt((int)gImitatedSounds.size()));
    return it->second;
}

void Parrot::playAmbientSound(Level* level, boost::shared_ptr<Entity> entity)
{
    if (!level || !entity || entity->isSilent())
        return;
    Random* random = level->getRandomFor(0, 0, 0);
    if (random == NULL || imitateNearbyMobs(level, entity.get()) || random->nextInt(200) != 0)
        return;
    entity->playSound(getAmbient(random), 1.0f, (float)getPitch(random));
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
    DATA_VARIANT = SynchedEntityData::defineId<int>(0x2d6c0003ULL, 0x2d6c0000ULL,
                                                    EntityDataSerializers::INT);
}

void Parrot::setVariant(int variant)
{
    mVariant = variant < 0 ? 0 : (variant > 4 ? 4 : variant);
    if (mEntityData != NULL && DATA_VARIANT != NULL)
        mEntityData->set(DATA_VARIANT, mVariant, true);
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
    mGoalSelector.addGoal(0, new SitGoal(this), true);
    mGoalSelector.addGoal(0, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(2, new FollowOwnerFlyingGoal(this, 1.0, 5.0f, 1.0f), true);
    mGoalSelector.addGoal(2, new WaterAvoidingRandomFlyingGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new LandOnOwnersShoulderGoal(this), true);
    mGoalSelector.addGoal(3, new FollowMobGoal(this, 1.0, 3.0f, 10.0f), true);
}

void Parrot::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
    getAttribute(SharedMonsterAttributes::FLYING_SPEED)->setBaseValue(0.4);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12.0);
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
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool feedingTameFood = isTameFood(item);

    if (!isTame() && feedingTameFood)
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);

        if (!isSilent())
        {
            mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PARROT_EAT,
                              getSoundSource(), 1.0f, (float)getPitch(mRandom), 16.0f);
        }

        if (!mLevel->mIsClientSide)
        {
            if (mRandom->nextInt(10) == 0)
            {
                tame(player);
                spawnTamingParticles(true);
                mLevel->broadcastEntityEvent(shared_from_this(), 7, 0);
            }
            else
            {
                spawnTamingParticles(false);
                mLevel->broadcastEntityEvent(shared_from_this(), 6, 0);
            }
        }

        return true;
    }

    if (item->getItem() == Items::cookie)
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);

        addEffect(new MobEffectInstance(MobEffects::POISON, 900), boost::shared_ptr<Entity>());

        if (player->isCreative() || !isInvulnerable())
            hurt(DamageSource::CreatePlayerAttack(player), 3.4028234663852886e+38f);

        return true;
    }

    if (!mLevel->mIsClientSide && !isFlying() && isTame() && isOwnedBy(player))
    {
        bool renaming = false;
        if (Item::getId(item->getItem()) == 421 && !isPassenger() && item->hasCustomHoverName())
        {
            if (!hasCustomName() || item->getHoverName() != getCustomName())
                renaming = true;
        }

        if (!renaming && !isLeashedToPlayer(player)
            && (item->isEmpty() || item->getItem() != Items::spawnEgg))
            getSitGoal()->wantToSit(!isSitting());
    }

    return Animal::mobInteract(player, hand);
}

bool Parrot::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return isTameFood(itemInstance);
}

bool Parrot::checkSpawnRules()
{
    BlockPos pos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY), Mth::floor(mZ));
    Block* block = mLevel->getBlockState(pos.below())->getBlock();
    if (block != NULL && dynamic_cast<LeafBlock*>(block) != NULL)
        return true;
    if (block == Blocks::grass)
        return true;
    if (block != NULL && dynamic_cast<LogBlock*>(block) != NULL)
        return true;
    if (block == Blocks::AIR && mLevel->getDaytimeRawBrightness(pos) >= 9 && Animal::checkSpawnRules())
        return true;
    return false;
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
    if (isLeashedToPlayer(holder.m_pPlayer))
        return 0x551269B5U;

    not_null_ptr<ItemInstance> item = holder.m_pItemInstance;
    if (item->getItem() == NULL)
        return 0xffffffffU;

    boost::shared_ptr<Player> player(holder.m_pPlayer,
                                     (Player*)holder.m_pPlayer.get());

    if (!isBaby())
    {
        if (Item::getId(item->getItem()) == 421)
        {
            if (item->hasCustomHoverName())
            {
                bool showRename = true;
                if (isTame())
                {
                    if (hasCustomName() && item->getHoverName() == getCustomName())
                        showRename = false;
                }
                if (showRename)
                    return 0x5592C692U;
            }

            if (!isOwnedBy(player))
                return 0xffffffffU;
            if (isSitting())
                return 0xE1122504U;
            if (!isPassenger())
                return 0x3102263BU;
            return 0xffffffffU;
        }
    }

    if (!isTame())
    {
        if (isTameFood(item) && getPassengers()->empty())
            return 0x95CFA728U;
        return 0xffffffffU;
    }

    if (Item::getId(item->getItem()) == 420 && !isLeashed())
        return 0xA1B34A96U;

    if (!isOwnedBy(player))
        return 0xffffffffU;
    if (isSitting())
        return 0xE1122504U;
    if (!isPassenger())
        return 0x3102263BU;
    return 0xffffffffU;
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
    return SoundEvents::PARROT_HURT;
}

const SoundEvent* Parrot::getDeathSound()
{
    return SoundEvents::PARROT_DEATH;
}

void Parrot::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::PARROT_STEP, 0.15000000596046448f, 1.0f);
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
    return mEntityData != NULL && DATA_VARIANT != NULL ? mEntityData->get(DATA_VARIANT) : mVariant;
}

void Parrot::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    if (mEntityData != NULL && DATA_VARIANT != NULL)
        mEntityData->createDateItem(DATA_VARIANT, 0);
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

ResourceLocation* Parrot::getDefaultLootTable()
{
    return &BuiltInLootTables::PARROT;
}

unsigned long long Parrot::GetType()
{
    return 0x000000002D6C0003ULL;
}

bool Parrot::NotParrotPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && entity->GetType() != 0x2d6c000300000000ULL;
}

Parrot::NotParrotPredicate::~NotParrotPredicate()
{
}
