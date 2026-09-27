#include "game/core/Dolphin.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/item/Item.hpp"
#include "game/core/GenericStats.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/BreathAirGoal.hpp"
#include "game/entity/ai/DolphinLookControl.hpp"
#include "game/entity/ai/DolphinMoveControl.hpp"
#include "game/entity/ai/FindUnderwaterTreasureGoal.hpp"
#include "game/entity/ai/FollowBoatGoal.hpp"
#include "game/entity/ai/FollowFlockLeaderGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/JumpGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveToWaterGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/ai/SwimWithMobGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/FishFoodItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

Dolphin::Dolphin(Level* level)
    : AgableWaterAnimal(level), mTreasureTime(0), mAirSupply(0), mTreasureGoal(NULL)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 0.6f);
    delete mMoveControl;
    mMoveControl = new DolphinMoveControl(this);
    delete mLookControl;
    mLookControl = new DolphinLookControl(this, 10);
    setCanPickUpLoot(false);
    mAirSupply = mRandom->nextInt(2, 5);
}

Dolphin::~Dolphin()
{
}

MobGroupData* Dolphin::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    setAirSupply(getMaxAirSupply());
    mTreasureTime = 400;
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

unsigned int Dolphin::getMaxAirSupply() { return 4800; }
double Dolphin::getEyeHeight() { return 1.2; }
int Dolphin::getAmbientSoundInterval() { return 120; }
int Dolphin::getMaxHeadXRot() { return 1; }
int Dolphin::getMaxHeadYRot() { return 1; }

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
        BlockPos above(mX, mY + 1.0, mZ);
        if (mLevel->getBlockState(above)->getMaterial() == Material::AIR)
        {
            setAirSupply(getMaxAirSupply());
        }
    }
    if (!isInWaterOrRain())
    {
        --mTreasureTime;
        if (mTreasureTime < 1)
        {
            hurt(NULL, 1.0f);
        }
    }
    else
    {
        mTreasureTime = 400;
    }
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
ResourceLocation* Dolphin::getDefaultLootTable() { return &BuiltInLootTables::DOLPHIN; }

bool Dolphin::doHurtTarget(boost::shared_ptr<Entity> target)
{
    EntityDamageSource* source =
        DamageSource::CreateMobAttack(boost::static_pointer_cast<Mob>(shared_from_this()));
    bool hurtTarget = target->hurt(source,
        (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
    delete source;
    if (hurtTarget)
    {
        doEnchantDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), target);
        playSound(SoundEvents::DOLPHIN_ATTACK, 1.0f, 1.0f);
    }
    return hurtTarget;
}

void Dolphin::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    Entity::positionRider(passenger);
    double angle = (double)mYRot * 0.01745329238474369;
    passenger->setPos(mX - std::sin(angle) * 0.4, mY + (double)getEyeHeight() + passenger->getRideHeight(),
                      mZ + std::cos(angle) * 0.4);
}

boost::shared_ptr<ItemEntity> Dolphin::drop(const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty()) return boost::shared_ptr<ItemEntity>();
    boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(mLevel, mX, mY - 0.3 + getEyeHeight(), mZ, item));
    itemEntity->setPickUpDelay(40);
    itemEntity->setThrower(getUUID());
    return itemEntity;
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
    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    if (!item->isEmpty() && isItemABribingFish(item))
        return IDS_TOOLTIPS_FEED;
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
    unsigned long long* swimWithMobTypes = new unsigned long long[1];
    swimWithMobTypes[0] = EntityType::PLAYER;
    unsigned long long* hurtByTargetTypes = new unsigned long long[2];
    hurtByTargetTypes[0] = EntityType::GUARDIAN;
    hurtByTargetTypes[1] = EntityType::ELDER_GUARDIAN;

    mGoalSelector.addGoal(0, new BreathAirGoal(this), true);
    mGoalSelector.addGoal(0, new MoveToWaterGoal(this, 1.0, 15, 5, 0.5), true);
    mTreasureGoal = new FindUnderwaterTreasureGoal(this, 2.0f, 30, 50);
    mGoalSelector.addGoal(2, mTreasureGoal, true);
    mGoalSelector.addGoal(2, new SwimWithMobGoal(this, 1.2f, 16.0f, 20,
        arrayWithLength<unsigned long long>(swimWithMobTypes, 1)), true);
    mGoalSelector.addGoal(3, new FollowFlockLeaderGoal(this, true), true);
    mGoalSelector.addGoal(4, new RandomSwimmingGoal(this, 1.0, 10, 10, 7), true);
    mGoalSelector.addGoal(4, new RandomLookAroundGoal(this), true);
    mGoalSelector.addGoal(5, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(5, new JumpGoal(this, 10), true);
    mGoalSelector.addGoal(6, new MeleeAttackGoal(this, 1.2, true), true);
    mGoalSelector.addGoal(8, new FollowBoatGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true,
        arrayWithLength<unsigned long long>(hurtByTargetTypes, 2)), true);
}

void Dolphin::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.2f);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(3.0);
}

PathNavigation* Dolphin::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, true);
}

bool Dolphin::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->isEmpty() || !isItemABribingFish(item))
        return AgableMob::mobInteract(player, hand);

    if (!mLevel->mIsClientSide)
    {
        mTreasureGoal->setBribed();
        playSound(SoundEvents::DOLPHIN_EAT, 1.0f, 1.0f);
        player->awardStat(GenericStats::echolocation(), GenericStats::param_echolocation());
    }

    int fishId = Item::getId(Items::fish);
    for (int i = 0; i < 7; ++i)
    {
        const double xa = mRandom->nextGaussian() * 0.02;
        const double ya = mRandom->nextGaussian() * 0.02;
        const double za = mRandom->nextGaussian() * 0.02;
        const float xOffset = mRandom->nextFloat() * mBbWidth * 2.0f;
        const float yOffset = mRandom->nextFloat() * mBbHeight;
        const float zOffset = mRandom->nextFloat() * mBbWidth * 2.0f;
        mLevel->addParticle(PARTICLE_ICON_CRACK, mX + xOffset - mBbWidth, mY + 0.5 + yOffset, mZ + zOffset - mBbWidth,
                            xa, ya, za, arrayWithLength<int>(&fishId, 1));
    }

    if (!player->mAbilities.mInstabuild)
        item->shrink(1);
    return true;
}

bool Dolphin::canHoldItem(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == Items::egg && isBaby() && isPassenger()) return false;
    return Mob::canHoldItem(item);
}

bool Dolphin::canRide(boost::shared_ptr<Entity>) { return false; }
const SoundEvent* Dolphin::getHurtSound(const DamageSource*) { return SoundEvents::DOLPHIN_HURT; }
const SoundEvent* Dolphin::getDeathSound() { return SoundEvents::DOLPHIN_DEATH; }
const SoundEvent* Dolphin::getAmbientSound() { return isInWater() ? SoundEvents::DOLPHIN_AMBIENT_WATER : SoundEvents::DOLPHIN_AMBIENT; }
const SoundEvent* Dolphin::getSwimSound() { return SoundEvents::DOLPHIN_SWIM; }
const SoundEvent* Dolphin::getSwimSplashSound() { return SoundEvents::DOLPHIN_SPLASH; }
bool Dolphin::generatesBubbles() { return false; }
unsigned long long Dolphin::GetType() { return EntityType::DOLPHIN; }
void Dolphin::create(Level* level) { new Dolphin(level); }
