#include "game/entity/ai/SkeletonTrapGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DifficultyInstance.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

SkeletonTrapGoal::SkeletonTrapGoal(SkeletonHorse* horse)
    : mHorse(horse)
{
}

SkeletonTrapGoal::~SkeletonTrapGoal()
{
}

bool SkeletonTrapGoal::canUse()
{
    return mHorse->mLevel->hasNearPlayer(mHorse->mX, mHorse->mY, mHorse->mZ, 10.0);
}

boost::shared_ptr<Skeleton> SkeletonTrapGoal::createSkeleton(
    const DifficultyInstance& difficulty, const boost::shared_ptr<AbstractHorse>& horse)
{
    boost::shared_ptr<Skeleton> skeleton(new Skeleton(mHorse->mLevel));
    skeleton->finalizeMobSpawn(difficulty, NULL, std::wstring());
    skeleton->setPos(horse->mX, horse->mY, horse->mZ);
    skeleton->mNoActionTime = 60;
    skeleton->setPersistenceRequired();

    not_null_ptr<ItemInstance> item = skeleton->getItemSlot(EquipmentSlot::MAINHAND);
    if (item->isEmpty())
    {
        item = not_null_ptr<ItemInstance>(new ItemInstance(Items::ironSword));
        skeleton->setItemSlot(EquipmentSlot::MAINHAND, item);
    }

    not_null_ptr<ItemInstance> mainHand = skeleton->getItemSlot(EquipmentSlot::MAINHAND);
    EnchantmentHelper::enchantItem(
        skeleton->getRandom(), mainHand,
        (int)(5.0F + difficulty.getSpecialMultiplier() *
              (float)skeleton->getRandom()->nextInt(18)), false);
    skeleton->setItemSlot(EquipmentSlot::MAINHAND, mainHand);
    return skeleton;
}

boost::shared_ptr<AbstractHorse> SkeletonTrapGoal::createHorse(const DifficultyInstance& difficulty)
{
    boost::shared_ptr<SkeletonHorse> horse(new SkeletonHorse(mHorse->mLevel));
    horse->finalizeMobSpawn(difficulty, NULL, std::wstring());
    horse->setPos(mHorse->mX, mHorse->mY, mHorse->mZ);
    horse->setTrap(true);
    horse->setTamed(true);
    horse->setAge(0);
    mHorse->mLevel->addEntity(horse);
    return horse;
}

void SkeletonTrapGoal::tick()
{
    BlockPos position(mHorse);
    DifficultyInstance difficulty = mHorse->mLevel->getCurrentDifficultyAt(position);

    mHorse->setTrap(false);
    mHorse->setTamed(true);
    mHorse->setAge(0);

    boost::shared_ptr<LightningBolt> lightning(new LightningBolt(
        mHorse->mLevel, mHorse->mX, mHorse->mY, mHorse->mZ, true));
    mHorse->mLevel->addEntity(lightning);

    boost::shared_ptr<AbstractHorse> horse = createHorse(difficulty);
    boost::shared_ptr<Skeleton> skeleton = createSkeleton(difficulty, horse);
    skeleton->startRiding(horse);

    int count = 3;
    while (count-- != 0)
    {
        horse = createHorse(difficulty);
        skeleton = createSkeleton(difficulty, horse);
        skeleton->startRiding(horse);
        horse->setPos(mHorse->mX + mHorse->getRandom()->nextGaussian() * 0.5,
                      mHorse->mY + 1.75,
                      mHorse->mZ + mHorse->getRandom()->nextGaussian() * 0.5);
    }
}

std::wstring SkeletonTrapGoal::getName()
{
    return std::wstring(L"SkeletonTrap");
}
