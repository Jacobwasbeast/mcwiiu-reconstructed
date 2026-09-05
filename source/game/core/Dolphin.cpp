#include "game/core/Dolphin.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FindUnderwaterTreasureGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/FishFoodItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"

Dolphin::Dolphin(Level* level)
    : AgableWaterAnimal(level), mTreasureTime(0), mAirSupply(0), mTreasureGoal(NULL)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 0.6f);
    setCanPickUpLoot(false);
    mAirSupply = mRandom->nextInt(2, 5);
}

Dolphin::~Dolphin()
{
}

MobGroupData* Dolphin::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    getMaxAirSupply();
    mTreasureTime = 400;
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

unsigned int Dolphin::getMaxAirSupply() { return 4800; }
double Dolphin::getEyeHeight() { return 1.2; }
unsigned int Dolphin::getAmbientSoundInterval() { return 120; }
unsigned int Dolphin::getMaxHeadXRot() { return 1; }
unsigned int Dolphin::getMaxHeadYRot() { return 1; }

void Dolphin::aiStep()
{
    if (isInWater() && getMaxHeadXRot())
    {
        mLevel->addParticle(PARTICLE_BUBBLE, false, mX + (mRandom->nextFloat() - 0.5f) * mBbWidth,
                            mY + mRandom->nextFloat() * mBbHeight,
                            mZ + (mRandom->nextFloat() - 0.5f) * mBbWidth,
                            0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    if (mAirSupply < 1 && !isPassenger() && mRandom->nextInt(40) == 0)
    {
        playSound(SoundEvents::DOLPHIN_BLOWHOLE, 1.0f, mRandom->nextFloat(0.8f, 1.2f));
        mAirSupply = 40;
    }
    Mob::aiStep();
}

void Dolphin::tick()
{
    Mob::tick();
    if (!mLevel->mIsClientSide)
    {
        BlockPos above(shared_from_this());
        if (mLevel->isWaterAt(above))
            getMaxAirSupply();
    }
    if (!isInWaterOrRain())
    {
        --mTreasureTime;
        if (mTreasureTime < 1) hurt(NULL, 1.0f);
    }
    else mTreasureTime = 400;
}

bool Dolphin::isPushedByWater() { return false; }
bool Dolphin::isInSpawnableOcean()
{
    BlockPos pos(shared_from_this());
    return mLevel->isWaterAt(pos);
}

bool Dolphin::checkSpawnRules()
{
    return AgableWaterAnimal::checkSpawnRules() && mY > 45.0 &&
           mY < (double)mLevel->getSeaLevel() && isInSpawnableOcean();
}

bool Dolphin::canBeLeashed(const boost::shared_ptr<Player>&) { return false; }
const void* Dolphin::getDefaultLootTable() { return &BuiltInLootTables::DOLPHIN; }

bool Dolphin::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (target == NULL) return false;
    LivingEntity* living = dynamic_cast<LivingEntity*>(target.get());
    if (living == NULL) return false;
    AttributeInstance* attack = getAttribute(NULL);
    float damage = attack == NULL ? 0.0f : (float)attack->getValue();
    return target->hurt(NULL, damage);
}

void Dolphin::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    Entity::positionRider(passenger);
    double angle = (double)mYRot * 0.01745329238474369;
    passenger->setPos(mX - std::sin(angle) * 0.4, mY + (double)getEyeHeight() + passenger->getRideHeight(),
                      mZ + std::cos(angle) * 0.4);
}

void Dolphin::drop(const not_null_ptr<ItemInstance>& item)
{
    if (!item->isEmpty()) spawnAtLocation(item, 0.0f);
}

boost::shared_ptr<AgableMob> Dolphin::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new Dolphin(mLevel));
}

bool Dolphin::isItemABribingFish(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty() || item->getItem() != Items::fish) return false;
    FishFoodItem::Type* type = FishFoodItem::Type::getByItem(item);
    return type != NULL && (type->getId() == 0 || type->getId() == 1);
}

unsigned int Dolphin::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return AgableMob::GetInteractTooltip(tooltipData);
}

void Dolphin::addAdditonalSaveData(CompoundTag* tag)
{
    AgableMob::addAdditonalSaveData(tag);
    tag->putBoolean(L"bribed", mTreasureGoal != NULL && mTreasureGoal->getBribed());
}

void Dolphin::readAdditionalSaveData(CompoundTag* tag)
{
    AgableMob::readAdditionalSaveData(tag);
    if (tag->getBoolean(L"bribed") && mTreasureGoal != NULL) mTreasureGoal->setBribed();
}

void Dolphin::registerGoals()
{
    Mob::registerGoals();
    mTreasureGoal = new FindUnderwaterTreasureGoal(this, 1.0f, 30, 50);
    mGoalSelector.addGoal(2, mTreasureGoal, true);
}

void Dolphin::registerAttributes()
{
    Mob::registerAttributes();
    AttributeInstance* health = getAttribute(NULL);
    if (health != NULL) health->setBaseValue(5.0);
}

WaterBoundPathNavigation* Dolphin::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, true);
}

bool Dolphin::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (isItemABribingFish(item))
    {
        if (!mLevel->mIsClientSide && mTreasureGoal != NULL) mTreasureGoal->setBribed();
        item->shrink(1);
        return true;
    }
    return AgableMob::mobInteract(player, hand);
}

bool Dolphin::canHoldItem(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == Items::fish && isBaby() && isPassenger()) return false;
    return Mob::canHoldItem(item);
}

bool Dolphin::canRide(boost::shared_ptr<Entity>) { return false; }
const SoundEvent* Dolphin::getHurtSound(const DamageSource*) { return SoundEvents::DOLPHIN_HURT; }
const SoundEvent* Dolphin::getDeathSound() { return SoundEvents::DOLPHIN_DEATH; }
const SoundEvent* Dolphin::getAmbientSound() { return isInWater() ? SoundEvents::DOLPHIN_AMBIENT_WATER : SoundEvents::DOLPHIN_AMBIENT; }
const SoundEvent* Dolphin::getSwimSound() { return SoundEvents::DOLPHIN_SWIM; }
const SoundEvent* Dolphin::getSwimSplashSound() { return SoundEvents::DOLPHIN_SPLASH; }
bool Dolphin::generatesBubbles() { return false; }
unsigned long long Dolphin::GetType() { return 0x0d602001ULL; }
void Dolphin::create(Level* level) { new Dolphin(level); }
