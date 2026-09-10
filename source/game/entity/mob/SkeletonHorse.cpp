#include "game/entity/mob/SkeletonHorse.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

SkeletonHorse::SkeletonHorse(Level* level)
    : AbstractHorse(level), mSkeletonTrapGoal(this), mTrap(false), mTrapTime(0)
{
    fjDerivedCtorCalls();
}

SkeletonHorse::~SkeletonHorse()
{
}

void SkeletonHorse::create(Level* level)
{
    new SkeletonHorse(level);
}

void SkeletonHorse::setTrap(bool trap)
{
    if (mTrap == trap)
    {
        return;
    }

    mTrap = trap;
    if (mTrap)
    {
        mGoalSelector.addGoal(1, &mSkeletonTrapGoal, false);
    }
    else
    {
        mGoalSelector.removeGoal(&mSkeletonTrapGoal);
    }
}

void SkeletonHorse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(15.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
}

void SkeletonHorse::registerGoals()
{
    AbstractHorse::registerGoals();
}

void SkeletonHorse::playUnderwaterSound(const BlockPos& pos, Block* block)
{
    if (isInWater())
    {
        if (!isVehicle())
        {
            playSound(SoundEvents::SKELETON_HORSE_STEP_WATER, 1.0f, 0.8f);
        }
        else
        {
            ++mTailCounter;
            if (mTailCounter < 6)
            {
                playSound(SoundEvents::SKELETON_HORSE_STEP_WATER, 1.0f, 0.8f);
                Entity::playUnderwaterSound(pos, block);
                return;
            }
            if (mTailCounter % 3 == 0)
            {
                playSound(SoundEvents::SKELETON_HORSE_GALLOP_WATER, 1.0f, 0.8f);
                Entity::playUnderwaterSound(pos, block);
                return;
            }
        }
    }
    Entity::playUnderwaterSound(pos, block);
}

void SkeletonHorse::playGallopSound(const SoundType* sound)
{
    playSound(SoundEvents::SKELETON_HORSE_GALLOP_WATER,
              sound->getVolume() * 0.15f, sound->getPitch());
}

const SoundEvent* SkeletonHorse::getJumpSound()
{
    return isInWater() ? SoundEvents::SKELETON_HORSE_JUMP_WATER : SoundEvents::HORSE_JUMP;
}

const SoundEvent* SkeletonHorse::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return isInWater() ? SoundEvents::SKELETON_HORSE_AMBIENT_WATER : SoundEvents::SKELETON_HORSE_AMBIENT;
}

const SoundEvent* SkeletonHorse::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::SKELETON_HORSE_DEATH;
}

const SoundEvent* SkeletonHorse::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::SKELETON_HORSE_HURT;
}

EMobType SkeletonHorse::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

double SkeletonHorse::getRideHeight()
{
    return Entity::getRideHeight() - 0.1875;
}

ResourceLocation* SkeletonHorse::getDefaultLootTable()
{
    return &BuiltInLootTables::SKELETON_HORSE;
}

boost::shared_ptr<AgableMob> SkeletonHorse::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<SkeletonHorse> child(new SkeletonHorse(mLevel));
    child->setOffspringAttributes(boost::static_pointer_cast<AgableMob>(shared_from_this()),
                                  boost::static_pointer_cast<AbstractHorse>(partner));
    return child;
}

bool SkeletonHorse::isTamed()
{
    return true;
}

bool SkeletonHorse::isTrap()
{
    return mTrap;
}

void SkeletonHorse::aiStep()
{
    AbstractHorse::aiStep();
    if (isTrap())
    {
        ++mTrapTime;
        if (mTrapTime > 17999)
        {
            remove();
        }
    }
}

void SkeletonHorse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, 0x2D690002ULL);
}

void SkeletonHorse::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractHorse::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"SkeletonTrap"), isTrap());
    tag->putInt(std::wstring(L"SkeletonTrapTime"), mTrapTime);
}

void SkeletonHorse::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractHorse::readAdditionalSaveData(tag);
    setTrap(tag->getBoolean(std::wstring(L"SkeletonTrap")));
    mTrapTime = tag->getInt(std::wstring(L"SkeletonTrapTime"));
}

bool SkeletonHorse::breathesWater()
{
    return true;
}

bool SkeletonHorse::mobInteract(const boost::shared_ptr<Player>& player,
                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    if (!item->isEmpty() && item->getItem() == Items::saddle)
    {
        return AbstractHorse::mobInteract(player, hand);
    }
    if (!isVehicle() && !item->isEmpty())
    {
        return false;
    }
    AbstractHorse::doPlayerRide(player);
    return true;
}

unsigned int SkeletonHorse::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return AgableMob::GetInteractTooltip(holder);
}

unsigned long long SkeletonHorse::GetType()
{
    return 0x2D690002ULL;
}
