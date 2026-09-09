#include "game/core/Wolf.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/BegGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowOwnerGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/NonTameRandomTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtTargetGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/PreySelector.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Llama.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/FoodItem.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<float>* Wolf::DATA_HEALTH = NULL;
const EntityDataAccessor<bool>* Wolf::DATA_INTERESTED = NULL;
const EntityDataAccessor<int>* Wolf::DATA_COLLAR_COLOR = NULL;

static const unsigned long long ENTITY_TYPE_WOLF = 0x000000000D6C0002ULL;
static const unsigned long long ENTITY_TYPE_TAMABLE_ANIMAL = 0x000000000D6C0000ULL;
static const unsigned long long ENTITY_TYPE_CREEPER = 0x000000005D540041ULL;
static const unsigned long long ENTITY_TYPE_GHAST = 0x000000005D0000C1ULL;
static const unsigned long long ENTITY_TYPE_ABSTRACT_HORSE = 0x000000000D690000ULL;
static const unsigned long long ENTITY_TYPE_LLAMA = 0x000000002D698003ULL;
static const unsigned long long ENTITY_TYPE_ARROW = 0x000000008A020000ULL;

static Predicate<boost::shared_ptr<Entity> >* WOLF_PREY_SELECTOR = new PreySelector();

Wolf::Wolf(Level* level)
    : TamableAnimal(level), mInterestedAngle(0.0f), mInterestedAngleO(0.0f),
      mIsWet(false), mIsShaking(false), mShakeAnim(0.0f), mShakeAnimO(0.0f)
{
    fjDerivedCtorCalls();
    setSize(0.6000000238418579f, 0.8500000238418579f);
}

Wolf::~Wolf() { }

void Wolf::create(Level* level) { new Wolf(level); }

void Wolf::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer, ENTITY_TYPE_WOLF); }

void Wolf::staticCtor()
{
    DATA_HEALTH = SynchedEntityData::defineId<float>(ENTITY_TYPE_WOLF, ENTITY_TYPE_TAMABLE_ANIMAL,
                                                     EntityDataSerializers::FLOAT);
    DATA_INTERESTED = SynchedEntityData::defineId<bool>(ENTITY_TYPE_WOLF, ENTITY_TYPE_TAMABLE_ANIMAL,
                                                        EntityDataSerializers::BOOLEAN);
    DATA_COLLAR_COLOR = SynchedEntityData::defineId<int>(ENTITY_TYPE_WOLF, ENTITY_TYPE_TAMABLE_ANIMAL,
                                                         EntityDataSerializers::INT);
}

void Wolf::registerGoals()
{
    delete mSitGoal;
    mSitGoal = new SitGoal(this);

    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, mSitGoal, false);
    mGoalSelector.addGoal(3, new WolfAvoidEntityGoal(this, typeid(Llama), 24.0f, 1.5, 1.5), true);
    mGoalSelector.addGoal(4, new LeapAtTargetGoal(this, 0.4000000059604645f), true);
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
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Turtle), false, Turtle::BABY_ON_LAND_SELECTOR, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false), true);
}

void Wolf::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.30000001192092896);
    if (isTame())
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(20.0);
    else
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(2.0);
}

void Wolf::setAngry(bool angry)
{
    unsigned char flags = mEntityData->get(TamableAnimal::DATA_FLAGS);
    if (angry)
        mEntityData->set(TamableAnimal::DATA_FLAGS, static_cast<unsigned char>(flags | 2), true);
    else
        mEntityData->set(TamableAnimal::DATA_FLAGS, static_cast<unsigned char>(flags & 0xfd), true);
}

void Wolf::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    if (target == NULL)
        setAngry(false);
    else if (!isTame())
        setAngry(true);
}

void Wolf::customServerAiStep()
{
    mEntityData->set(DATA_HEALTH, getHealth(), true);
}

void Wolf::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    mEntityData->createDateItem(DATA_HEALTH, getHealth());
    mEntityData->createDateItem(DATA_INTERESTED, false);
    mEntityData->createDateItem(DATA_COLLAR_COLOR, DyeColor::RED->getItemData());
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
    tag->putByte(L"CollarColor", static_cast<unsigned char>(getCollarColor()->getItemData()));
}

void Wolf::setCollarColor(const DyeColor* color)
{
    mEntityData->set(DATA_COLLAR_COLOR, color->getItemData() & 0xf, true);
}

void Wolf::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setAngry(tag->getBoolean(L"Angry"));
    if (tag->contains(L"CollarColor"))
        setCollarColor(DyeColor::byItemData(tag->getByte(L"CollarColor")));
}

const SoundEvent* Wolf::getAmbientSound()
{
    if (isAngry())
        return SoundEvents::WOLF_GROWL;

    if (mRandom->nextInt(3) == 0)
    {
        if (isTame() && mEntityData->get(DATA_HEALTH) < 10.0f)
            return SoundEvents::WOLF_WHINE;
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
    if (!mLevel->mIsClientSide && mIsWet && !mIsShaking && !isPathFinding() && mOnGround)
    {
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
        mIsShaking = true;
        mLevel->broadcastEntityEvent(shared_from_this(), 8, 0);
    }

    if (!mLevel->mIsClientSide && (getTarget() == NULL || !getTarget()->isAlive()) && isAngry())
        setAngry(false);
}

bool Wolf::isInterested() { return mEntityData->get(DATA_INTERESTED); }

void Wolf::tick()
{
    PIXBeginNamedEvent(0.0f, "Wolf tick");
    Mob::tick();
    mInterestedAngleO = mInterestedAngle;

    if (isInterested())
        mInterestedAngle += (1.0f - mInterestedAngle) * 0.4000000059604645f;
    else
        mInterestedAngle += (0.0f - mInterestedAngle) * 0.4000000059604645f;

    if (isInWaterRainOrBubble())
    {
        mShakeAnim = 0.0f;
        mIsWet = true;
        mShakeAnimO = 0.0f;
        mIsShaking = false;
    }
    else if (mIsShaking)
    {
        if (mShakeAnim == 0.0f)
        {
            playSound(SoundEvents::WOLF_SHAKE, getSoundVolume(),
                      (mRandom->nextFloat() - mRandom->nextFloat()) * 0.20000000298023224f + 1.0f);
        }

        mShakeAnimO = mShakeAnim;
        mShakeAnim += 0.05000000074505806f;
        if (mShakeAnimO >= 2.0f)
        {
            mShakeAnimO = 0.0f;
            mShakeAnim = 0.0f;
            mIsWet = false;
            mIsShaking = false;
        }

        if (mShakeAnim > 0.4000000059604645f)
        {
            float baseY = static_cast<float>(mBoundingBox->mMinY);
            int count = static_cast<int>(Mth::sin((mShakeAnim - 0.4000000059604645f) * 3.1415927410125732f) * 7.0f);
            for (int i = 0; i < count; ++i)
            {
                float xOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                float zOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                mLevel->addParticle(PARTICLE_SPLASH, mX + static_cast<double>(xOffset),
                                    static_cast<double>(baseY + 0.800000011920929f),
                                    mZ + static_cast<double>(zOffset), mXd, mYd, mZd,
                                    arrayWithLength<int>());
            }
        }
    }

    if (Entity::instanceof(getTarget(), ENTITY_TYPE_PLAYER) && mLevel->getDifficulty() == Difficulty::PEACEFUL)
        setTarget(boost::shared_ptr<LivingEntity>());

    PIXEndNamedEvent();
}

double Wolf::getBodyRollAngle(float partialTick, float offset)
{
    float shake = ((mShakeAnim - mShakeAnimO) * partialTick + mShakeAnimO + offset) / 1.7999999523162842f;
    if (shake < 0.0f)
        shake = 0.0f;
    else if (shake > 1.0f)
        shake = 1.0f;

    return Mth::sin(shake * 3.1415927410125732f)
         * Mth::sin(shake * 3.1415927410125732f * 11.0f)
         * 0.15000000596046448f * 3.1415927410125732f;
}

double Wolf::getHeadRollAngle(float partialTick)
{
    return ((mInterestedAngle - mInterestedAngleO) * partialTick + mInterestedAngleO)
         * 0.15000000596046448f * 3.1415927410125732f;
}

double Wolf::getEyeHeight() { return mBbHeight * 0.800000011920929f; }

int Wolf::getMaxHeadXRot()
{
    if (isSitting())
        return 20;
    return Mob::getMaxHeadXRot();
}

bool Wolf::hurt(const DamageSource* source, float amount)
{
    if (isTame())
    {
        boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
        if (directEntity != NULL && (directEntity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            if (!boost::static_pointer_cast<Player>(directEntity)->canHarmPlayer(getOwnerUUID()))
                return false;
        }
    }

    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (mSitGoal != NULL)
        mSitGoal->wantToSit(false);

    if (attacker != NULL
        && (attacker->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER
        && (attacker->GetType() & ENTITY_TYPE_ARROW) != ENTITY_TYPE_ARROW)
    {
        amount = (amount + 1.0f) * 0.5f;
    }

    return Animal::hurt(source, amount);
}

bool Wolf::doHurtTarget(boost::shared_ptr<Entity> target)
{
    EntityDamageSource* source =
        DamageSource::CreateMobAttack(boost::static_pointer_cast<Mob>(shared_from_this()));
    float damage = static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
    bool hurtTarget = target->hurt(source, damage);
    delete source;
    if (hurtTarget)
        doEnchantDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), target);
    return hurtTarget;
}

void Wolf::setTame(bool tame)
{
    TamableAnimal::setTame(tame);
    if (tame)
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(20.0);
    else
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4.0);
}

void Wolf::tame(boost::shared_ptr<Player> player, bool showParticles, bool)
{
    TamableAnimal::tame(player);
    mNavigation->stop();
    setTarget(boost::shared_ptr<LivingEntity>());
    setHealth(20.0f);
    spawnTamingParticles(showParticles);
}

bool Wolf::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (!isTame())
    {
        if (Item::getId(item->getItem()) == Item::getId(Items::bone) && !isAngry())
        {
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);

            if (!mLevel->mIsClientSide)
            {
                if (mRandom->nextInt(3) == 0)
                {
                    player->awardStat(gStats->get_tamedEntity(ENTITY_TYPE_WOLF),
                                      GenericStats::param_tamedEntity(ENTITY_TYPE_WOLF));
                    tame(player, true, true);
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

        if (!AgeUpBaby(player, item))
            return AgableMob::mobInteract(player, hand);
        return true;
    }

    if (!item->isEmpty())
    {
        FoodItem* food = dynamic_cast<FoodItem*>(item->getItem());
        if (food == NULL)
        {
            if (item->getItem() == Items::dye)
            {
                const DyeColor* color = DyeColor::byItemData(item->getAuxValue());
                if (color != getCollarColor())
                {
                    setCollarColor(color);
                    if (!player->mAbilities.mInstabuild)
                        item->shrink(1);
                    return true;
                }
            }
        }
        else if (food->isMeat())
        {
            if (mEntityData->get(DATA_HEALTH) < 20.0f)
            {
                if (!player->mAbilities.mInstabuild)
                    item->shrink(1);

                heal(static_cast<float>(food->getNutrition(item)));
                if (item->getCount() < 1)
                    player->mInventory->setItem(player->mInventory->m_selected, ItemInstance::EMPTY);
                return true;
            }
        }
    }

    if (isOwnedBy(player) && !mLevel->mIsClientSide && !isFood(item) && !isLeashedToPlayer(player)
        && item->getItem() != Items::lead
        && !(!item->isEmpty() && item->getItem() == Items::spawnEgg && item->getAuxValue() == 0x5f))
    {
        bool renaming = false;
        if (Item::getId(item->getItem()) == 421 && !isPassenger() && item->hasCustomHoverName())
        {
            if (!hasCustomName() || item->getHoverName() != getCustomName())
                renaming = true;
        }

        if (!renaming)
        {
            mSitGoal->wantToSit(!isSitting());
            mJumping = false;
            mNavigation->stop();
            setTarget(boost::shared_ptr<LivingEntity>());
        }
    }

    return Animal::mobInteract(player, hand);
}

void Wolf::handleEntityEvent(unsigned char event, int data)
{
    if (event == 8)
    {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
        return;
    }
    TamableAnimal::handleEntityEvent(event, data);
}

double Wolf::getTailAngle()
{
    if (isAngry())
        return 1.539380431175232f;

    if (isTame())
    {
        return (0.550000011920929f - (getMaxHealth() - mEntityData->get(DATA_HEALTH)) * 0.019999999552965164f)
             * 3.1415927410125732f;
    }

    return 0.6283185482025146f;
}

bool Wolf::isFood(const not_null_ptr<ItemInstance>& item)
{
    if (dynamic_cast<FoodItem*>(item->getItem()) == NULL)
        return false;
    return dynamic_cast<FoodItem*>(item->getItem())->isMeat();
}

int Wolf::getMaxSpawnClusterSize() { return 4; }

int Wolf::GetSynchedHealth() { return static_cast<int>(getEntityData()->get(DATA_HEALTH)); }

unsigned int Wolf::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return 0x551269B5;

    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case 0x15F:
        if (!isTame())
            return 0xFFFFFFFF;
        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue()) != getCollarColor())
            return 0x58AE5A1A;
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return 0xFFFFFFFF;
        if (isSitting())
            return 0xE1122504;
        return 0x3102263B;

    case 0x160:
        if (!isAngry() && !isTame())
        {
            if (!getPassengers()->empty())
                return 0xFFFFFFFF;
            return 0x95CFA728;
        }
        if (isPassenger() || !isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return 0xFFFFFFFF;
        if (isSitting())
            return 0xE1122504;
        return 0x3102263B;

    case 0x170:
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return 0xFFFFFFFF;
        if (isSitting())
            return 0xE1122504;
        return 0x3102263B;

    case 0x17F:
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == ENTITY_TYPE_WOLF)
            return 0x4C36CD96;
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return 0xFFFFFFFF;
        if (isSitting())
            return 0xE1122504;
        return 0x3102263B;

    case 0x1A4:
        if (isLeashed() || !isOwnedBy(tooltipData.m_pPlayer))
            return 0xFFFFFFFF;
        return 0xA1B34A96;

    case 0x1A5:
        if (isPassenger())
            return 0xFFFFFFFF;
        if (tooltipData.m_pItemInstance->hasCustomHoverName()
            && (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName() != getCustomName()))
        {
            return 0x5592C692;
        }
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return 0xFFFFFFFF;
        if (isSitting())
            return 0xE1122504;
        return 0x3102263B;

    default:
        break;
    }

    if (isBaby() && isFood(tooltipData.m_pItemInstance))
        return 0x82F8D90F;

    if (isTame())
    {
        if (!isFood(tooltipData.m_pItemInstance))
        {
            if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            {
                if (isSitting())
                    return 0xE1122504;
                return 0x3102263B;
            }
        }
        else
        {
            if (GetSynchedHealth() < 20)
                return 0x56B3F98D;
            if (!isInLove() && !isBaby() && !isBreedCoolingDown())
                return 0xB47A2AB6;
        }
    }

    return 0xFFFFFFFF;
}

boost::shared_ptr<AgableMob> Wolf::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    boost::shared_ptr<Wolf> offspring(new Wolf(mLevel));
    if (!getOwnerUUID().empty())
    {
        offspring->setOwnerUUID(getOwnerUUID());
        offspring->setTame(true);
        offspring->setHealth(offspring->getMaxHealth());
        offspring->setCollarColor(getCollarColor());
    }
    return offspring;
}

void Wolf::setIsInterested(bool interested)
{
    mEntityData->set(DATA_INTERESTED, interested, true);
}

bool Wolf::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
        return false;
    if (!isTame())
        return false;
    if (other->GetType() != ENTITY_TYPE_WOLF)
        return false;

    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(other);
    if (!wolf->isTame())
        return false;
    if (wolf->isSitting())
        return false;

    return isInLove() && wolf->isInLove();
}

bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner)
{
    if (target->GetType() == ENTITY_TYPE_CREEPER)
        return false;
    if (target->GetType() == ENTITY_TYPE_GHAST)
        return false;

    if (target->GetType() == ENTITY_TYPE_WOLF)
    {
        boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(target);
        if (wolf->isTame() && wolf->getOwner() == owner)
            return false;
    }

    if ((target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && (owner->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && !boost::static_pointer_cast<Player>(owner)->canHarmPlayer(boost::static_pointer_cast<Player>(target)))
    {
        return false;
    }

    if (target->GetType() == ENTITY_TYPE_ABSTRACT_HORSE
        && boost::static_pointer_cast<AbstractHorse>(target)->isTamed())
    {
        return false;
    }

    return true;
}

bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return !isAngry() && TamableAnimal::canBeLeashed(player);
}

unsigned long long Wolf::GetType() { return ENTITY_TYPE_WOLF; }

Wolf::WolfAvoidEntityGoal::WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType,
                                               float maxDistance, double walkSpeedModifier,
                                               double sprintSpeedModifier)
    : AvoidEntityGoal(wolf, avoidType, maxDistance, walkSpeedModifier, sprintSpeedModifier), mWolf(wolf) { }

Wolf::WolfAvoidEntityGoal::~WolfAvoidEntityGoal() { }

bool Wolf::WolfAvoidEntityGoal::avoidLlama(const boost::shared_ptr<Llama>& llama)
{
    return mWolf->mRandom->nextInt(5) <= llama->getStrength();
}

bool Wolf::WolfAvoidEntityGoal::canUse()
{
    if (!AvoidEntityGoal::canUse())
        return false;

    boost::shared_ptr<Entity> toAvoid = mToAvoid.lock();
    if (toAvoid->GetType() != ENTITY_TYPE_LLAMA)
        return false;
    if (mWolf->isTame())
        return false;

    return avoidLlama(boost::static_pointer_cast<Llama>(toAvoid));
}

void Wolf::WolfAvoidEntityGoal::start()
{
    mWolf->setTarget(boost::shared_ptr<LivingEntity>());
    AvoidEntityGoal::start();
}

void Wolf::WolfAvoidEntityGoal::tick()
{
    mWolf->setTarget(boost::shared_ptr<LivingEntity>());
    AvoidEntityGoal::tick();
}
