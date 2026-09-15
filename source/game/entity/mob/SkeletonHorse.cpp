#include "game/entity/mob/SkeletonHorse.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"

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
    mGoalSelector.removeGoal(mFloatGoal);
    mGoalSelector.removeGoal(mWaterAvoidingRandomStrollGoal);
    mGoalSelector.addGoal(6, new SinkInWaterRandomStrollGoal(this, 0.7), true);
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
    if (isInWater())
        playSound(SoundEvents::SKELETON_HORSE_GALLOP_WATER, sound->getVolume() * 0.15f, sound->getPitch());
    else
        playSound(SoundEvents::HORSE_GALLOP, sound->getVolume() * 0.15f, sound->getPitch());
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

boost::shared_ptr<AgableMob> SkeletonHorse::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new SkeletonHorse(mLevel));
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
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool notEmpty = !item->isEmpty();

    if (notEmpty && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);

    if (!isTamed() && notEmpty)
        return false;

    if (isLeashedToPlayer(player))
        return false;

    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);

    if (player->isSneaking())
    {
        openInventory(player);
        return true;
    }

    if (isVehicle())
        return AbstractHorse::mobInteract(player, hand);

    if (notEmpty)
    {
        if (item->getItem() == Items::saddle && !isSaddled())
        {
            openInventory(player);
            return true;
        }
        if (item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
            return true;
    }

    doPlayerRide(player);
    return true;
}

unsigned int SkeletonHorse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return 0xffffffffU;

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    unsigned int itemId = Item::getId(item->getItem());
    bool horseArmor = itemId >= 417 && itemId <= 419 || itemId == 453;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == 420)
    {
        if (isLeashedToPlayer(player))
            return 0x551269b5U;
        if (isLeashed())
            return 0xffffffffU;
        return 0xa1b34a96U;
    }

    if (itemId == 383)
    {
        if (SpawnEggItem::fjGetType(item) == GetType())
            return 0x4c36cd96U;
        if (isLeashedToPlayer(player))
            return 0x551269b5U;
        return 0xffffffffU;
    }

    if (isLeashedToPlayer(player))
        return 0x551269b5U;

    if (itemId == 421)
    {
        if (!player->isSneaking() || !isTamed())
        {
            if (!item->hasCustomHoverName())
                return 0xffffffffU;
            if (hasCustomName() && item->getHoverName() == getCustomName())
                return 0xffffffffU;
            return 0x5592c692U;
        }
        if (!item->hasCustomHoverName())
        {
            if (getPassengers()->empty())
                return 0x497adcf0U;
            return 0xffffffffU;
        }
        if (hasCustomName() && item->getHoverName() == getCustomName())
            return 0xffffffffU;
        return 0x5592c692U;
    }

    if (isBaby())
        return 0xffffffffU;

    bool openInventory = true;
    if (!player->isSneaking())
    {
        if ((itemId != 329 || isSaddled()) && (!wearsArmor() || !horseArmor))
            openInventory = false;
    }
    else if (isVehicle())
    {
        if (!mobInteract(player, InteractionHand::eInteractionHand_MainHand))
        {
            if ((itemId != 329 || isSaddled()) && (!wearsArmor() || !horseArmor))
                openInventory = false;
        }
    }

    if (openInventory)
        return 0x41590ad1U;

    if (getPassengers()->empty())
        return 0x497adcf0U;

    return 0xffffffffU;
}

unsigned long long SkeletonHorse::GetType()
{
    return 0x2D690002ULL;
}
