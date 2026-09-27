#include "game/core/Llama.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/LlamaFollowCaravanGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"


const EntityDataAccessor<int>* Llama::DATA_STRENGTH = NULL;
const EntityDataAccessor<int>* Llama::DATA_SWAG = NULL;
const EntityDataAccessor<int>* Llama::DATA_VARIANT = NULL;

Llama::LlamaDefendTargetGoal::LlamaDefendTargetGoal(Llama* llama)
    : NearestAttackableTargetGoal(llama, typeid(LivingEntity), 10, true, false, NULL, false, false) { }
Llama::LlamaDefendTargetGoal::~LlamaDefendTargetGoal() { }
bool Llama::LlamaDefendTargetGoal::canUse()
{
    if (!NearestAttackableTargetGoal::canUse())
        return false;

    double followDistance = getFollowDistance();
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(LivingEntity),
                                         getTargetSearchArea(followDistance));
    boost::shared_ptr<LivingEntity> target;
    if (entities != NULL && !entities->empty())
    {
        const double maxDistSq = followDistance * followDistance;
        double bestDistSq = maxDistSq;
        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            const double distSq = mMob->distanceToSqr(entities->at(i));
            if (distSq <= bestDistSq)
            {
                target = boost::dynamic_pointer_cast<LivingEntity>(entities->at(i));
                bestDistSq = distSq;
            }
        }
    }
    delete entities;

    if (target && target->GetType() == EntityType::WOLF)
    {
        if (!boost::static_pointer_cast<Wolf>(target)->isAngry())
            return true;
    }

    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    return false;
}
double Llama::LlamaDefendTargetGoal::getFollowDistance() { return TargetGoal::getFollowDistance() * 0.25; }

Llama::LlamaGroupData::LlamaGroupData(int variant) : MobGroupData(), mVariant(variant) { }
Llama::LlamaGroupData::~LlamaGroupData() { }

Llama::LlamaHurtByTargetGoal::LlamaHurtByTargetGoal(Llama* llama)
    : HurtByTargetGoal(llama, false, arrayWithLength<unsigned long long>()) { }
Llama::LlamaHurtByTargetGoal::~LlamaHurtByTargetGoal() { }
bool Llama::LlamaHurtByTargetGoal::canContinueToUse()
{
    Llama* llama = static_cast<Llama*>(mMob);
    if (llama->GetType() == EntityType::LLAMA && llama->mDidSpit)
    {
        llama->setDidSpit(false);
        return false;
    }

    return TargetGoal::canContinueToUse();
}

Llama::Llama(Level* level)
    : AbstractChestedHorse(level), mDidSpit(false), mStrength(1), mVariant(0), mCaravanHead(), mCaravanTail()
{
    AbstractHorse::fjDerivedCtorCalls();
    setSize(0.9f, 1.87f);
    internalSetSize(1.0f);
}
Llama::~Llama() { }
void Llama::create(Level* level) { new Llama(level); }
const DyeColor* Llama::getSwag()
{
    if (getEntityData() == NULL || DATA_SWAG == NULL)
    {
        return NULL;
    }

    int blockData = getEntityData()->get(DATA_SWAG);
    return blockData == -1 ? NULL : DyeColor::byBlockData(blockData);
}
void Llama::containerChanged(Container* container) { AbstractChestedHorse::containerChanged(container); updateEquipment(); }
void Llama::spit(const boost::shared_ptr<LivingEntity>& target)
{
    boost::shared_ptr<Llama> owner = boost::static_pointer_cast<Llama>(shared_from_this());
    boost::shared_ptr<LlamaSpit> spitEntity(new LlamaSpit(mLevel, owner));
    double dx = target->mX - mX;
    double dz = target->mZ - mZ;
    double targetY = target->mBoundingBox->mMinY + (double)target->mEyeHeight / 3.0;
    double distance = Mth::sqrt(dx * dx + dz * dz);
    spitEntity->shoot(dx, targetY - spitEntity->mY + distance * 0.2, dz, 1.5f, 10.0f);
    playSound(SoundEvents::LLAMA_SPIT, 1.0f,
              (getRandom()->nextFloat() - getRandom()->nextFloat()) * 0.2f + 1.0f);
    mLevel->addEntity(spitEntity);
    mDidSpit = true;
}
void Llama::performRangedAttack(boost::shared_ptr<LivingEntity> target, float) { spit(target); }
bool Llama::isAggressive() { return false; }
void Llama::setAggressive(bool) { }
void Llama::staticCtor()
{
    DATA_STRENGTH = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                      EntityDataSerializers::INT);
    DATA_SWAG = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                 EntityDataSerializers::INT);
    DATA_VARIANT = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                    EntityDataSerializers::INT);
}
void Llama::setStrength(int strength)
{
    mStrength = strength < 1 ? 1 : (strength > 5 ? 5 : strength);
    if (getEntityData() != NULL && DATA_STRENGTH != NULL)
    {
        getEntityData()->set(DATA_STRENGTH, mStrength, true);
    }
}
void Llama::setRandomStrength() { setStrength(getRandom()->nextInt(3) + 1); }
int Llama::getStrength()
{
    return getEntityData() != NULL && DATA_STRENGTH != NULL ? getEntityData()->get(DATA_STRENGTH) : mStrength;
}
int Llama::getVariant()
{
    int variant = getEntityData() != NULL && DATA_VARIANT != NULL ? getEntityData()->get(DATA_VARIANT) : mVariant;
    return variant < 0 ? 0 : (variant > 3 ? 3 : variant);
}
void Llama::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractChestedHorse::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    tag->putInt(L"Strength", getStrength());

    not_null_ptr<ItemInstance> item = mInventory->getItem(1);
    if (!item->isEmpty())
    {
        not_null_ptr<ItemInstance> decorItem = mInventory->getItem(1);
        tag->put(L"DecorItem", decorItem->save(new CompoundTag()));
    }
}
void Llama::setVariant(int variant)
{
    mVariant = variant;
    if (getEntityData() != NULL && DATA_VARIANT != NULL)
    {
        getEntityData()->set(DATA_VARIANT, variant, true);
    }
}
void Llama::readAdditionalSaveData(CompoundTag* tag)
{
    setStrength(tag->getInt(L"Strength"));
    AbstractChestedHorse::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
    if (tag->contains(L"DecorItem", Tag::TAG_COMPOUND))
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(tag->getCompound(L"DecorItem")));
        mInventory->setItem(1, item);
    }
}
void Llama::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new RunAroundLikeCrazyGoal(this, 1.2), true);
    mGoalSelector.addGoal(2, new LlamaFollowCaravanGoal(this, 1.25), true);
    mGoalSelector.addGoal(3, new RangedAttackGoal(this, this, 0.7, 40, 20.0f), true);
    mGoalSelector.addGoal(3, new PanicGoal(this, 1.2), true);
    mGoalSelector.addGoal(4, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(5, new FollowParentGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.7), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Mob), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new LlamaHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new LlamaDefendTargetGoal(this), true);
}
void Llama::registerAttributes()
{
    AbstractChestedHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(40.0);
}
void Llama::defineSynchedData()
{
    AbstractChestedHorse::defineSynchedData();
    getEntityData()->createDateItem(DATA_STRENGTH, 0);
    getEntityData()->createDateItem(DATA_SWAG, -1);
    getEntityData()->createDateItem(DATA_VARIANT, 0);
}
int Llama::getInventorySize()
{
    if (hasChest())
        return getInventoryColumns() * 3 + 2;
    return AbstractChestedHorse::getInventorySize();
}
void Llama::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
        return;

    if (!RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
    {
        const float angle = mYBodyRot * 0.01745329238474369f;
        const float sin = Mth::sin(angle);
        const float cos = Mth::cos(angle);
        passenger->setPos(mX + 0.30000001192092896 * sin,
                          mY + getRideHeight() + passenger->getRidingHeight(),
                          mZ - 0.30000001192092896 * cos);
    }
}
double Llama::getRideHeight() { return AbstractChestedHorse::getRideHeight(); }
bool Llama::canBeControlledByRider() { return false; }
int Llama::handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item)
{
    float healAmount = 0.0f;
    int flag = 0;
    int growth = 0;
    int temperGain = 0;
    Item* food = item->getItem();

    if (food == Items::wheat)
    {
        healAmount = 2.0f;
        growth = 10;
        temperGain = 3;
    }
    else if (food == Item::byBlock(Blocks::hay_block))
    {
        healAmount = 10.0f;
        growth = 90;
        temperGain = 6;
        if (isTamed() && !isBaby())
            flag = CheckAndSetInLove(player);
    }

    if (getHealth() < getMaxHealth() && healAmount > 0.0f)
    {
        heal(healAmount);
        flag = 1;
    }

    if (isBaby() && growth != 0)
    {
        ageUp(growth, true);
        flag = 1;
    }

    if (temperGain == 0)
    {
        if (flag == 0)
            return 0;
    }
    else
    {
        if (flag == 0 && isTamed())
            return 0;
        if (getTemper() >= getMaxTemper())
        {
            if (flag == 0)
                return 0;
        }
        else
        {
            flag = 1;
            modifyTemper(temperGain);
        }
    }

    if (!isSilent())
        eating(Item::getId(food));

    return flag;
}
bool Llama::isImmobile()
{
    if (getHealth() > 0.0f && !isEating())
        return false;
    return true;
}
MobGroupData* Llama::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    MobGroupData* result = AbstractHorse::finalizeMobSpawn(difficulty, data, name);
    setRandomStrength();
    LlamaGroupData* group = dynamic_cast<LlamaGroupData*>(result);
    if (group == NULL)
    {
        int variant = getRandom()->nextInt(4);
        group = new LlamaGroupData(variant);
        result = group;
        setVariant(variant);
    }
    else
    {
        setVariant(group->mVariant);
    }
    return result;
}
bool Llama::hasSwag() { return getSwag() != NULL; }
const SoundEvent* Llama::getAngrySound() { return SoundEvents::LLAMA_ANGRY; }
const SoundEvent* Llama::getAmbientSound() { return SoundEvents::LLAMA_AMBIENT; }
const SoundEvent* Llama::getHurtSound(const DamageSource*) { return SoundEvents::LLAMA_HURT; }
const SoundEvent* Llama::getDeathSound() { return SoundEvents::LLAMA_DEATH; }
void Llama::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::LLAMA_STEP, 0.15f, 1.0f);
}
void Llama::playChestEquipsSound()
{
    playSound(SoundEvents::DONKEY_CHEST, 1.0f,
              1.0f + (getRandom()->nextFloat() - getRandom()->nextFloat()) * 0.2f);
}
void Llama::makeMad()
{
    const SoundEvent* sound = getAngrySound();
    if (sound != NULL)
    {
        playSound(sound, getSoundVolume(), getVoicePitch());
    }
}
ResourceLocation* Llama::getDefaultLootTable() { return &BuiltInLootTables::LLAMA; }
int Llama::getInventoryColumns() { return getStrength(); }
bool Llama::wearsArmor() { return true; }
bool Llama::isArmor(const not_null_ptr<ItemInstance>& item)
{
    return item->getItem() == Item::byBlock(Blocks::carpet);
}
bool Llama::canBeSaddled() const { return false; }
unsigned int Llama::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return IDS_NONE;

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    unsigned int itemId = Item::getId(item->getItem());
    bool food = itemId == BlockId::HAY_BLOCK || itemId == ItemId::WHEAT;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == ItemId::SPAWN_EGG)
    {
        if (SpawnEggItem::fjGetType(item) == GetType())
            return IDS_TOOLTIPS_PLACE;
        if (isLeashedToPlayer(player))
            return IDS_TOOLTIPS_UNLEASH;
        return IDS_NONE;
    }

    if (isLeashedToPlayer(player))
        return IDS_TOOLTIPS_UNLEASH;

    if (itemId == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (isTamed() && getPassengers()->empty())
            return IDS_TOOLTIPS_MOUNT;
        return IDS_NONE;
    }

    if (itemId == ItemId::NAME_TAG && (!player->isSneaking() || !isTamed()))
    {
        if (!item->hasCustomHoverName())
            return IDS_NONE;
        if (hasCustomName() && item->getHoverName() == getCustomName())
            return IDS_NONE;
        return IDS_TOOLTIPS_NAME;
    }

    if (!isBaby())
    {
        if (isTamed())
        {
            if (player->isSneaking()
                && (!player->isVehicle()
                    || mobInteract(player, InteractionHand::eInteractionHand_MainHand)))
            {
                return getPassengers()->empty() ? IDS_TOOLTIPS_MOUNT : IDS_NONE;
            }

            if (itemId == ItemId::SADDLE && !isSaddled())
                return IDS_TOOLTIPS_OPEN;
            if (itemId == BlockId::CHEST && !hasChest())
                return IDS_TOOLTIPS_ATTACH;
            if (itemId == BlockId::HAY_BLOCK && !isInLove() && getPassengers()->empty() && !isBreedCoolingDown())
                return IDS_TOOLTIPS_LOVEMODE;
            if (food && getHealth() < getMaxHealth())
                return IDS_TOOLTIPS_HEAL;
            if (getPassengers()->empty())
                return IDS_TOOLTIPS_MOUNT;
            return IDS_NONE;
        }

        if (item->isEmpty() && getPassengers()->empty())
            return IDS_TOOLTIPS_TAME;
    }

    if (food)
        return IDS_TOOLTIPS_FEED;

    return IDS_NONE;
}
bool Llama::HideUseTooltips() { return true; }
void Llama::setSwag(const DyeColor* color)
{
    int blockData = -1;
    if (color != NULL)
    {
        blockData = color->getBlockData();
    }
    getEntityData()->set(DATA_SWAG, blockData, true);
}

void Llama::setSwagByItem(const not_null_ptr<ItemInstance>& item)
{
    if (item->isEmpty())
    {
        setSwag(NULL);
        return;
    }

    setSwag(DyeColor::byBlockData(item->getAuxValue()));
}
void Llama::updateEquipment()
{
    if (mLevel->mIsClientSide)
        return;

    AbstractHorse::updateEquipment();
    not_null_ptr<ItemInstance> item = mInventory->getItem(1);
    setSwagByItem(item);
}
int Llama::getMaxTemper() const { return 30; }
bool Llama::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
        return false;

    if (other->GetType() == EntityType::LLAMA && canParent())
    {
        boost::shared_ptr<Llama> partner = boost::static_pointer_cast<Llama>(other);
        return partner->canParent();
    }

    return false;
}
boost::shared_ptr<AgableMob> Llama::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Llama> baby(new Llama(mLevel));
    AbstractHorse::setOffspringAttributes(partner, baby);

    boost::shared_ptr<Llama> llamaPartner = boost::static_pointer_cast<Llama>(partner);
    int maxStrength = getStrength();
    int partnerStrength = llamaPartner->getStrength();
    if (partnerStrength > maxStrength)
        maxStrength = partnerStrength;

    int strength = getRandom()->nextInt(maxStrength);
    int newStrength = strength + 1;
    if (getRandom()->nextFloat() < 0.03f)
        newStrength = strength + 2;
    baby->setStrength(newStrength);

    if (getRandom()->nextBoolean())
        baby->setVariant(getVariant());
    else
        baby->setVariant(llamaPartner->getVariant());

    return baby;
}
void Llama::setDidSpit(bool didSpit) { mDidSpit = didSpit; }
void Llama::causeFallDamage(float fallDistance, float damageMultiplier)
{
    const float scaled = (fallDistance * 0.5f - 3.0f) * damageMultiplier;
    int damage = static_cast<int>(scaled);
    if (static_cast<float>(damage) < scaled)
        ++damage;

    if (static_cast<float>(damage) <= 0.0f)
        return;

    if (fallDistance >= 6.0f)
    {
        hurt(DamageSource::FALL, static_cast<float>(damage));
        if (isVehicle())
        {
            std::set<boost::shared_ptr<Entity> >* passengers = getIndirectPassengers();
            for (std::set<boost::shared_ptr<Entity> >::iterator it = passengers->begin();
                 it != passengers->end(); ++it)
            {
                (*it)->hurt(DamageSource::FALL, static_cast<float>(damage));
            }
            delete passengers;
        }
    }

    Block* block = mLevel->getBlockState(BlockPos(mX, mY - 0.2 - static_cast<double>(mYRotO), mZ))->getBlock();
    if (block->mMaterial == Material::AIR || isSilent())
        return;

    const SoundType* soundType = block->mSoundType;
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, soundType->getStepSound(),
                      getSoundSource(), soundType->getVolume() * 0.5f,
                      soundType->getPitch() * 0.75f, 16.0f);
}
void Llama::leaveCaravan()
{
    boost::shared_ptr<Llama> head = mCaravanHead.lock();
    if (head)
        head->mCaravanTail.reset();
    mCaravanHead.reset();
}

void Llama::joinCaravan(const boost::shared_ptr<Llama>& llama)
{
    mCaravanHead = llama;
    llama->mCaravanTail = boost::static_pointer_cast<Llama>(shared_from_this());
}
bool Llama::hasCaravanTail()
{
    boost::weak_ptr<Llama> weakTail(mCaravanTail);
    return weakTail.lock().get() != NULL;
}

bool Llama::inCaravan()
{
    boost::weak_ptr<Llama> weakHead(mCaravanHead);
    return weakHead.lock().get() != NULL;
}
boost::shared_ptr<Llama> Llama::getCaravanHead() { return mCaravanHead.lock(); }
double Llama::followLeashSpeed() { return 2.0; }
void Llama::followMommy()
{
    if (!inCaravan() && isBaby())
    {
        AbstractHorse::followMommy();
    }
}
bool Llama::canEatGrass() const { return false; }
unsigned long long Llama::GetType() { return EntityType::LLAMA; }
