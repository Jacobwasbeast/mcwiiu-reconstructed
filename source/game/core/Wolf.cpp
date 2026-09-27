#include "game/world/generation/MultiplayerLocalPlayer.hpp"
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
#include "game/entity/ai/SitGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Llama.hpp"
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
      mWetShade(0.0f)
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
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Player), true, NULL, false), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Animal), false, NULL, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
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
    unsigned char flags = getEntityData()->get(DATA_FLAGS);
    getEntityData()->set(DATA_FLAGS, angry ? static_cast<unsigned char>(flags | 2)
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
    TamableAnimal::customServerAiStep();
    getEntityData()->set(DATA_HEALTH_ID, getHealth(), true);
}
void Wolf::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    getEntityData()->createDateItem(DATA_HEALTH_ID, 1.0f);
    getEntityData()->createDateItem(DATA_INTERESTED, false);
    getEntityData()->createDateItem(DATA_COLLAR_COLOR, DyeColor::RED->getItemData());
}
void Wolf::playStepSound(const BlockPos&, Block*) { playSound(SoundEvents::WOLF_STEP, 0.15f, 1.0f); }
bool Wolf::isAngry() { return (getEntityData()->get(DATA_FLAGS) & 2) != 0; }
const DyeColor* Wolf::getCollarColor() { return DyeColor::byItemData(getEntityData()->get(DATA_COLLAR_COLOR)); }
void Wolf::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Angry", isAngry());
    tag->putByte(L"CollarColor", static_cast<unsigned char>(getCollarColor()->getItemData()));
}
void Wolf::setCollarColor(const DyeColor* color) { if (color != NULL) getEntityData()->set(DATA_COLLAR_COLOR, color->getItemData(), true); }
void Wolf::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setAngry(tag->getBoolean(L"Angry"));
    if (tag->contains(L"CollarColor"))
        setCollarColor(DyeColor::byItemData(tag->getByte(L"CollarColor")));
}
const SoundEvent* Wolf::getAmbientSound() { return isAngry() ? SoundEvents::WOLF_GROWL : SoundEvents::WOLF_AMBIENT; }
const SoundEvent* Wolf::getHurtSound(const DamageSource*) { return SoundEvents::WOLF_HURT; }
const SoundEvent* Wolf::getDeathSound() { return SoundEvents::WOLF_DEATH; }
float Wolf::getSoundVolume() { return 0.4f; }
ResourceLocation* Wolf::getDefaultLootTable() { return &BuiltInLootTables::WOLF; }
void Wolf::aiStep()
{
    Animal::aiStep();
    if (!mLevel->mIsClientSide && isTame() && !mIsShaking &&
        getTarget() == NULL && isInWaterRainOrBubble()) {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mWetShade = 0.0f;
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SHAKE_WETNESS, 0);
    }
    if (!mLevel->mIsClientSide && getTarget() == NULL && isAngry())
        setAngry(false);
}
bool Wolf::isInterested() { return getEntityData()->get(DATA_INTERESTED); }
void Wolf::tick()
{
    Mob::tick();
    mInterestedAngleO = mInterestedAngle;
    mInterestedAngle += isInterested() ? (1.0f - mInterestedAngle) * 0.4f
                                       : -mInterestedAngle * 0.4f;
    if (isInWaterRainOrBubble()) {
        mShakeAnim = 0.0f;
        mIsWet = true;
        mWetShade = 0.0f;
        mIsShaking = false;
    } else if (mIsShaking) {
        mShakeAnim += 0.05f;
        if (mShakeAnim >= 2.0f) {
            mIsWet = false;
            mIsShaking = false;
            mWetShade = 0.0f;
            mShakeAnim = 0.0f;
        } else {
            mWetShade = mShakeAnim;
        }
    } else {
        mWetShade = 0.0f;
        mShakeAnim = 0.0f;
    }
}
double Wolf::getBodyRollAngle(float partialTick, float bodyRotation)
{
    float shake = (mShakeAnim - mWetShade) * partialTick + mWetShade;
    if (shake < 0.0f) shake = 0.0f;
    if (shake > 1.0f) shake = 1.0f;
    const float angle = shake * 3.14159265358979323846f;
    return static_cast<double>(Mth::sin(angle) * Mth::sin(angle * 11.0f) * 0.15f);
}
double Wolf::getHeadRollAngle(float partialTick)
{
    return static_cast<double>(((mInterestedAngle - mInterestedAngleO) * partialTick + mInterestedAngleO) * 0.15f * 3.14159265358979323846f);
}
double Wolf::getEyeHeight() { return static_cast<double>(mBbHeight * 0.8f); }
int Wolf::getMaxHeadXRot() { return isInWaterRainOrBubble() ? 20 : Mob::getMaxHeadXRot(); }
bool Wolf::hurt(const DamageSource* source, float damage)
{
    if (source == NULL)
        return false;
    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker != NULL && attacker->GetType() == EntityType::WOLF && isTame())
        return false;
    const bool hurt = Animal::hurt(source, damage);
    if (hurt && getTarget() == NULL)
        setAngry(true);
    return hurt;
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
void Wolf::tame(boost::shared_ptr<Player> player, bool success, bool broadcast) { if (success) { TamableAnimal::tame(player); setAngry(false); } if (broadcast && mLevel != NULL) mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED, 0); }
bool Wolf::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (player == NULL)
        return false;
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!isTame() && !isAngry() && item->getItem() == Items::bone) {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);
        if (!mLevel->mIsClientSide)
            tame(player, getRandom()->nextInt(3) == 0, true);
        return true;
    }
    if (isTame() && !item->isEmpty()) {
        if (item->getItem() == Items::dye) {
            const DyeColor* color = DyeColor::byItemData(item->getAuxValue());
            if (color != getCollarColor()) {
                setCollarColor(color);
                if (!player->mAbilities.mInstabuild)
                    item->shrink(1);
                return true;
            }
        }
        if (isFood(item) && getHealth() < getMaxHealth()) {
            FoodItem* food = dynamic_cast<FoodItem*>(item->getItem());
            if (food != NULL)
                heal(static_cast<float>(food->getNutrition(item)));
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }
    }
    bool interacted = Animal::mobInteract(player, hand);
    if (interacted && isTame())
        setIsInterested(false);
    return interacted;
}
void Wolf::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::SHAKE_WETNESS) {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mWetShade = 0.0f;
        return;
    }
    TamableAnimal::handleEntityEvent(event, data);
}
double Wolf::getTailAngle() { return isAngry() ? 0.8 : 1.0 + mInterestedAngle * 0.1; }
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
bool Wolf::setIsInterested(bool interested) { getEntityData()->set(DATA_INTERESTED, interested, true); return interested; }
bool Wolf::canMate(const boost::shared_ptr<Animal>& other)
{
    boost::shared_ptr<Wolf> wolf = boost::dynamic_pointer_cast<Wolf>(other);
    return wolf != NULL && isTame() && wolf->isTame() && isInLove() && wolf->isInLove();
}
bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target)
{
    if (owner == NULL || target == NULL) return false;
    if (target->GetType() == EntityType::PLAYER && owner->GetType() == EntityType::PLAYER) return false;
    boost::shared_ptr<TamableAnimal> tamable = boost::dynamic_pointer_cast<TamableAnimal>(target);
    if (tamable != NULL && tamable->isTame() && tamable->isOwnedBy(owner)) return false;
    return true;
}
bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isAngry() && TamableAnimal::canBeLeashed(player); }
unsigned long long Wolf::GetType() { return EntityType::WOLF; }
int Wolf::getMaxSpawnClusterSize()
{
    return Mob::getMaxSpawnClusterSize();
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
