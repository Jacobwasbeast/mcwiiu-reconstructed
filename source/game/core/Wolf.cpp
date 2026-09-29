#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Difficulty.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Wolf.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/ItemId.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/BegGoal.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowOwnerGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Inventory.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Llama.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PreySelector.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/ai/NonTameRandomTargetGoal.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/core/DyeColor.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/FoodItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/world/Level.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/core/Mth.hpp"

static Predicate<boost::shared_ptr<Entity> >* PREY_SELECTOR = new PreySelector();
static const int WOLF_SPAWN_EGG_AUX = 95;

const EntityDataAccessor<float>* Wolf::DATA_HEALTH_ID = NULL;
const EntityDataAccessor<bool>* Wolf::DATA_INTERESTED = NULL;
const EntityDataAccessor<int>* Wolf::DATA_COLLAR_COLOR = NULL;
Wolf::Wolf(Level* level)
    : TamableAnimal(level),
      mInterestedAngle(0.0f),
      mInterestedAngleO(0.0f),
      mIsWet(false),
      mIsShaking(false),
      mShakeAnim(0.0f),
      mShakeAnimO(0.0f)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 0.85f);
}
Wolf::~Wolf() {}
void Wolf::create(Level* level) { new Wolf(level); }
void Wolf::staticCtor()
{
    DATA_HEALTH_ID = SynchedEntityData::defineId<float>(EntityType::WOLF,
                                                        EntityType::TAMABLE_ANIMAL,
                                                        EntityDataSerializers::FLOAT);
    DATA_INTERESTED = SynchedEntityData::defineId<bool>(EntityType::WOLF,
                                                        EntityType::TAMABLE_ANIMAL,
                                                        EntityDataSerializers::BOOLEAN);
    DATA_COLLAR_COLOR = SynchedEntityData::defineId<int>(EntityType::WOLF,
                                                         EntityType::TAMABLE_ANIMAL,
                                                         EntityDataSerializers::INT);
}
void Wolf::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer, EntityType::WOLF); }
void Wolf::registerGoals()
{
    delete mSitGoal;
    mSitGoal = new SitGoal(this);
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, mSitGoal, false);
    mGoalSelector.addGoal(3, new WolfAvoidEntityGoal(this, typeid(Llama), 24.0f, 1.5, 1.5), true);
    mGoalSelector.addGoal(4, new LeapAtTargetGoal(this, 0.4f), true);
    mGoalSelector.addGoal(5, new MeleeAttackGoal(this, 1.0, true), true);
    mGoalSelector.addGoal(6, new FollowOwnerGoal(this, 1.0, 10.0f, 2.0f), true);
    mGoalSelector.addGoal(7, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(9, new BegGoal(this, 8.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(10, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new OwnerHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new OwnerHurtTargetGoal(this), true);
    mTargetSelector.addGoal(3, new HurtByTargetGoal(this, true,
                                                      arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Animal), true, PREY_SELECTOR, false), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Turtle), false,
                                                           Turtle::BABY_ON_LAND_SELECTOR, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false), true);
}
void Wolf::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(isTame() ? 20.0 : 8.0);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(2.0);
}
void Wolf::setAngry(bool angry)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    mEntityData->set(DATA_FLAGS, angry ? static_cast<unsigned char>(flags | 2)
                                           : static_cast<unsigned char>(flags & 0xfd), true);
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
    mEntityData->set(DATA_HEALTH_ID, getHealth(), true);
}
void Wolf::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    mEntityData->createDateItem(DATA_HEALTH_ID, 1.0f);
    mEntityData->createDateItem(DATA_INTERESTED, false);
    mEntityData->createDateItem(DATA_COLLAR_COLOR, DyeColor::RED->getItemData());
}
void Wolf::playStepSound(const BlockPos&, Block*) { playSound(SoundEvents::WOLF_STEP, 0.15f, 1.0f); }
bool Wolf::isAngry() { return (mEntityData->get(DATA_FLAGS) & 2) != 0; }
const DyeColor* Wolf::getCollarColor() { return DyeColor::byItemData(mEntityData->get(DATA_COLLAR_COLOR)); }
void Wolf::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Angry", isAngry());
    tag->putByte(L"CollarColor", static_cast<unsigned char>(getCollarColor()->getItemData()));
}
void Wolf::setCollarColor(const DyeColor* color) { if (color != NULL) mEntityData->set(DATA_COLLAR_COLOR, color->getItemData(), true); }
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
        if (isTame() && mEntityData->get(DATA_HEALTH_ID) < 10.0f)
            return SoundEvents::WOLF_WHINE;
        return SoundEvents::WOLF_PANT;
    }
    return SoundEvents::WOLF_AMBIENT;
}
const SoundEvent* Wolf::getHurtSound(const DamageSource*) { return SoundEvents::WOLF_HURT; }
const SoundEvent* Wolf::getDeathSound() { return SoundEvents::WOLF_DEATH; }
float Wolf::getSoundVolume() { return 0.4f; }
ResourceLocation* Wolf::getDefaultLootTable() { return &BuiltInLootTables::WOLF; }
void Wolf::aiStep()
{
    Animal::aiStep();
    if (!mLevel->mIsClientSide && mIsWet && !mIsShaking && !isPathFinding() && mOnGround) {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SHAKE_WETNESS, 0);
    }
    if (!mLevel->mIsClientSide)
    {
        boost::shared_ptr<LivingEntity> target = getTarget();
        if ((target == NULL || !target->isAlive()) && isAngry())
            setAngry(false);
    }
}
bool Wolf::isInterested() { return mEntityData->get(DATA_INTERESTED); }
void Wolf::tick()
{
    PIXBeginNamedEvent(0.0f, "Wolf tick");
    Mob::tick();

    mInterestedAngleO = mInterestedAngle;
    if (isInterested())
    {
        mInterestedAngle += (1.0f - mInterestedAngle) * 0.4f;
    }
    else
    {
        mInterestedAngle += (0.0f - mInterestedAngle) * 0.4f;
    }

    if (isInWaterRainOrBubble())
    {
        mIsWet = true;
        mIsShaking = false;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
    }
    else if (mIsShaking)
    {
        if (mShakeAnim == 0.0f)
        {
            playSound(SoundEvents::WOLF_SHAKE, getSoundVolume(),
                      (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
        }

        mShakeAnimO = mShakeAnim;
        mShakeAnim += 0.05f;

        if (mShakeAnimO >= 2.0f)
        {
            mIsWet = false;
            mIsShaking = false;
            mShakeAnimO = 0.0f;
            mShakeAnim = 0.0f;
        }
        else if (mShakeAnim > 0.4f)
        {
            float y = (float)mBoundingBox->mMinY;
            int count = (int)(Mth::sin((mShakeAnim - 0.4f) * Mth::PI) * 7.0f);

            for (int i = 0; i < count; ++i)
            {
                float xOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                float zOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                mLevel->addParticle(PARTICLE_SPLASH, mX + xOffset, y + 0.8f, mZ + zOffset, mXd, mYd, mZd,
                                    arrayWithLength<int>());
            }
        }
    }

    if (Entity::instanceof(getTarget(), EntityType::PLAYER) && mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        setTarget(boost::shared_ptr<LivingEntity>());
    }

    PIXEndNamedEvent();
}
double Wolf::getBodyRollAngle(float partialTick, float bodyRotation)
{
    float shake = (mShakeAnimO + (mShakeAnim - mShakeAnimO) * partialTick + bodyRotation) / 1.8f;
    if (shake < 0.0f)
    {
        shake = 0.0f;
    }
    else if (shake > 1.0f)
    {
        shake = 1.0f;
    }
    return Mth::sin(shake * Mth::PI) * Mth::sin(shake * Mth::PI * 11.0f) * 0.15f * Mth::PI;
}
double Wolf::getHeadRollAngle(float partialTick)
{
    return static_cast<double>(((mInterestedAngle - mInterestedAngleO) * partialTick + mInterestedAngleO) * 0.15f * 3.14159265358979323846f);
}
double Wolf::getEyeHeight() { return static_cast<double>(mBbHeight * 0.8f); }
int Wolf::getMaxHeadXRot() { return isSitting() ? 20 : Mob::getMaxHeadXRot(); }
bool Wolf::hurt(const DamageSource* source, float damage)
{
    if (isTame())
    {
        boost::shared_ptr<Entity> direct = source->getDirectEntity();
        if (direct && (direct->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
        {
            if (!boost::static_pointer_cast<Player>(direct)->canHarmPlayer(getOwnerUUID()))
                return false;
        }
    }

    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (mSitGoal != NULL)
        mSitGoal->wantToSit(false);
    if (attacker && (attacker->GetType() & EntityType::PLAYER) != EntityType::PLAYER
        && (attacker->GetType() & EntityType::ARROW) != EntityType::ARROW)
        damage = (damage + 1.0f) * 0.5f;
    return Animal::hurt(source, damage);
}
bool Wolf::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (target == NULL)
        return false;
    boost::shared_ptr<LivingEntity> self =
        boost::static_pointer_cast<LivingEntity>(shared_from_this());
    EntityDamageSource* source = DamageSource::CreateMobAttack(self);
    const float damage = static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
    const bool hurtTarget = target->hurt(source, damage);
    if (hurtTarget)
        doEnchantDamageEffects(self, target);
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
            {
                item->shrink(1);
            }

            if (!mLevel->mIsClientSide)
            {
                if (mRandom->nextInt(3) == 0)
                {
                    player->awardStat(gStats->get_tamedEntity(EntityType::WOLF),
                                      GenericStats::param_tamedEntity(EntityType::WOLF));
                    tame(player, true, true);
                    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED, 0);
                }
                else
                {
                    spawnTamingParticles(false);
                    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_FAILED, 0);
                }
            }
            return true;
        }

        if (AgeUpBaby(player, item))
        {
            return true;
        }
        return AgableMob::mobInteract(player, hand);
    }

    if (!item->isEmpty())
    {
        FoodItem* food = dynamic_cast<FoodItem*>(item->getItem());
        if (food != NULL)
        {
            if (food->isMeat() && mEntityData->get(DATA_HEALTH_ID) < 20.0f)
            {
                if (!player->mAbilities.mInstabuild)
                {
                    item->shrink(1);
                }
                heal((float)food->getNutrition(item));
                if (item->getCount() < 1)
                {
                    player->mInventory->setItem(player->mInventory->m_selected, ItemInstance::EMPTY);
                }
                return true;
            }
        }
        else if (item->getItem() == Items::dye)
        {
            const DyeColor* color = DyeColor::byItemData(item->getAuxValue());
            if (color != getCollarColor())
            {
                setCollarColor(color);
                if (!player->mAbilities.mInstabuild)
                {
                    item->shrink(1);
                }
                return true;
            }
        }
    }

    if (isOwnedBy(player) && !mLevel->mIsClientSide && !isFood(item) && !isLeashedToPlayer(player)
        && item->getItem() != Items::lead
        && (item->isEmpty() || item->getItem() != Items::spawnEgg || item->getAuxValue() != WOLF_SPAWN_EGG_AUX))
    {
        bool renaming = false;
        if (Item::getId(item->getItem()) == ItemId::NAME_TAG && !isPassenger() && item->hasCustomHoverName())
        {
            renaming = !hasCustomName() || item->getHoverName() != getCustomName();
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
    if (event == EntityEvent::SHAKE_WETNESS) {
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
    {
        return 1.5393804f;
    }

    if (isTame())
    {
        return (0.55f - (getMaxHealth() - mEntityData->get(DATA_HEALTH_ID)) * 0.02f) * Mth::PI;
    }

    return Mth::PI / 5.0f;
}
bool Wolf::isFood(const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
        return false;
    FoodItem* food = dynamic_cast<FoodItem*>(item->getItem());
    return food != NULL && food->isMeat();
}
int Wolf::GetSynchedHealth() { return static_cast<int>(getEntityData()->get(DATA_HEALTH_ID)); }
unsigned int Wolf::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    bool baby;
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    case ItemId::DYE:
        if (!isTame())
            return IDS_NONE;
        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue()) != getCollarColor())
            return IDS_TOOLTIPS_DYECOLLAR;
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return IDS_NONE;
        return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
    case ItemId::BONE:
        if (!isAngry() && !isTame())
            return getPassengers()->empty() ? IDS_TOOLTIPS_TAME : IDS_NONE;
        if (isPassenger() || !isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return IDS_NONE;
        return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
    case ItemId::ENDER_PEARL:
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return IDS_NONE;
        return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
    case ItemId::SPAWN_EGG:
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::WOLF)
            return IDS_TOOLTIPS_PLACE;
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return IDS_NONE;
        return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
    case ItemId::LEAD:
        if (isLeashed() || !isOwnedBy(tooltipData.m_pPlayer))
            return IDS_NONE;
        return IDS_TOOLTIPS_LEASH;
    case ItemId::NAME_TAG:
        if (isPassenger())
            return IDS_NONE;
        if (tooltipData.m_pItemInstance->hasCustomHoverName()
            && (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0))
            return IDS_TOOLTIPS_NAME;
        if (!isOwnedBy(tooltipData.m_pPlayer) || isInWater())
            return IDS_NONE;
        return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
    default:
        baby = isBaby();
        break;
    }

    if (baby && isFood(tooltipData.m_pItemInstance))
        return IDS_TOOLTIPS_FEED;

    if (isTame())
    {
        if (!isFood(tooltipData.m_pItemInstance))
        {
            if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
                return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        }
        else
        {
            if (GetSynchedHealth() < 20)
                return IDS_TOOLTIPS_HEAL;
            if (!isInLove() && !isBaby() && !isBreedCoolingDown())
                return IDS_TOOLTIPS_LOVEMODE;
        }
    }
    return IDS_NONE;
}
boost::shared_ptr<AgableMob> Wolf::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Wolf> offspring(new Wolf(mLevel));
    boost::shared_ptr<Wolf> other = boost::dynamic_pointer_cast<Wolf>(partner);
    const std::wstring owner = getOwnerUUID();
    if (!owner.empty()) {
        offspring->setOwnerUUID(owner);
        offspring->setTame(true);
        offspring->setCollarColor(getCollarColor());
    } else if (other != NULL && !other->getOwnerUUID().empty()) {
        offspring->setOwnerUUID(other->getOwnerUUID());
        offspring->setTame(true);
        offspring->setCollarColor(other->getCollarColor());
    }
    return boost::static_pointer_cast<AgableMob>(offspring);
}
bool Wolf::setIsInterested(bool interested) { mEntityData->set(DATA_INTERESTED, interested, true); return interested; }
bool Wolf::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other.get() == this || !isTame() || other->GetType() != EntityType::WOLF)
        return false;
    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(other);
    if (!wolf->isTame() || wolf->isSitting())
        return false;
    return isInLove() && wolf->isInLove();
}
bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner)
{
    if (target->GetType() == EntityType::CREEPER || target->GetType() == EntityType::GHAST)
    {
        return false;
    }

    if (target->GetType() == EntityType::WOLF)
    {
        boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(target);
        if (wolf->isTame() && wolf->getOwner() == owner)
        {
            return false;
        }
    }

    if ((target->GetType() & EntityType::PLAYER) == EntityType::PLAYER
        && (owner->GetType() & EntityType::PLAYER) == EntityType::PLAYER
        && !boost::static_pointer_cast<Player>(owner)->canHarmPlayer(boost::static_pointer_cast<Player>(target)))
    {
        return false;
    }

    if (target->GetType() == EntityType::ABSTRACT_HORSE)
    {
        return !boost::static_pointer_cast<AbstractHorse>(target)->isTamed();
    }

    return true;
}
bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isAngry() && TamableAnimal::canBeLeashed(player); }
unsigned long long Wolf::GetType() { return EntityType::WOLF; }
int Wolf::getMaxSpawnClusterSize()
{
    return 4;
}

Wolf::WolfAvoidEntityGoal::WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType, float maxDistance, double walkSpeed, double sprintSpeed)
    : AvoidEntityGoal(wolf, avoidType, maxDistance, walkSpeed, sprintSpeed), mWolf(wolf) {}
Wolf::WolfAvoidEntityGoal::~WolfAvoidEntityGoal() {}
bool Wolf::WolfAvoidEntityGoal::avoidLlama(const boost::shared_ptr<Llama>& llama)
{
    return llama != NULL && mWolf->getRandom()->nextInt(5) <= llama->getStrength();
}
bool Wolf::WolfAvoidEntityGoal::canUse()
{
    if (!AvoidEntityGoal::canUse())
        return false;
    boost::shared_ptr<Entity> target = mToAvoid.lock();
    boost::shared_ptr<Llama> llama = boost::dynamic_pointer_cast<Llama>(target);
    return avoidLlama(llama);
}
void Wolf::WolfAvoidEntityGoal::start()
{
    boost::shared_ptr<LivingEntity> owner = mWolf->getOwner();
    AvoidEntityGoal::start();
}
void Wolf::WolfAvoidEntityGoal::tick()
{
    boost::shared_ptr<LivingEntity> owner = mWolf->getOwner();
    AvoidEntityGoal::tick();
}
