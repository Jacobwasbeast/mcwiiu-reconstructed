#include "game/core/Llama.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/BuiltInLootTables.hpp"
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
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

static const unsigned long long ENTITY_TYPE_LLAMA = 0x2d698003ULL;

const EntityDataAccessor<int>* Llama::DATA_STRENGTH = NULL;
const EntityDataAccessor<int>* Llama::DATA_SWAG = NULL;
const EntityDataAccessor<int>* Llama::DATA_VARIANT = NULL;

Llama::LlamaDefendTargetGoal::LlamaDefendTargetGoal(Llama* llama)
    : NearestAttackableTargetGoal(llama, typeid(LivingEntity), 10, true, false, NULL, false, false) { }
Llama::LlamaDefendTargetGoal::~LlamaDefendTargetGoal() { }
bool Llama::LlamaDefendTargetGoal::canUse() { return NearestAttackableTargetGoal::canUse(); }
double Llama::LlamaDefendTargetGoal::getFollowDistance() { return TargetGoal::getFollowDistance() * 0.25; }

Llama::LlamaGroupData::LlamaGroupData(int variant) : MobGroupData(), mVariant(variant) { }
Llama::LlamaGroupData::~LlamaGroupData() { }

Llama::LlamaHurtByTargetGoal::LlamaHurtByTargetGoal(Llama* llama)
    : HurtByTargetGoal(llama, false, arrayWithLength<unsigned long long>()) { }
Llama::LlamaHurtByTargetGoal::~LlamaHurtByTargetGoal() { }
bool Llama::LlamaHurtByTargetGoal::canContinueToUse()
{
    Llama* llama = static_cast<Llama*>(mMob);
    if (llama->GetType() == 0x2D698003ULL && llama->mDidSpit)
    {
        llama->setDidSpit(false);
        return false;
    }

    return TargetGoal::canContinueToUse();
}

Llama::Llama(Level* level)
    : AbstractChestedHorse(level), mDidSpit(false), mStrength(1), mVariant(0), mCaravanHead(), mCaravanTail() { }
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
void Llama::spit(const boost::shared_ptr<LivingEntity>&) { mDidSpit = true; }
void Llama::performRangedAttack(boost::shared_ptr<LivingEntity> target, float) { spit(target); }
bool Llama::isAggressive() { return false; }
void Llama::setAggressive(bool) { }
void Llama::staticCtor()
{
    DATA_STRENGTH = SynchedEntityData::defineId<int>(ENTITY_TYPE_LLAMA, 0x0d698000ULL,
                                                      EntityDataSerializers::INT);
    DATA_SWAG = SynchedEntityData::defineId<int>(ENTITY_TYPE_LLAMA, 0x0d698000ULL,
                                                 EntityDataSerializers::INT);
    DATA_VARIANT = SynchedEntityData::defineId<int>(ENTITY_TYPE_LLAMA, 0x0d698000ULL,
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
void Llama::addAdditonalSaveData(CompoundTag* tag) { AbstractChestedHorse::addAdditonalSaveData(tag); tag->putInt(L"Variant", getVariant()); tag->putInt(L"Strength", getStrength()); }
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
    AbstractChestedHorse::readAdditionalSaveData(tag);
    if (tag->contains(L"Variant"))
    {
        setVariant(tag->getInt(L"Variant"));
    }
    if (tag->contains(L"Strength"))
    {
        setStrength(tag->getInt(L"Strength"));
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
void Llama::registerAttributes() { AbstractChestedHorse::registerAttributes(); }
void Llama::defineSynchedData()
{
    AbstractChestedHorse::defineSynchedData();
    getEntityData()->createDateItem(DATA_STRENGTH, 0);
    getEntityData()->createDateItem(DATA_SWAG, -1);
    getEntityData()->createDateItem(DATA_VARIANT, 0);
}
int Llama::getInventorySize() { return getStrength() * 3; }
void Llama::positionRider(const boost::shared_ptr<Entity>& passenger) { AbstractHorse::positionRider(passenger); }
double Llama::getRideHeight() { return AbstractChestedHorse::getRideHeight(); }
bool Llama::canBeControlledByRider() { return false; }
int Llama::handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item) { return AbstractHorse::handleEating(player, item); }
bool Llama::isImmobile() { return AbstractHorse::isImmobile(); }
MobGroupData* Llama::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name) { return AbstractHorse::finalizeMobSpawn(difficulty, data, name); }
bool Llama::hasSwag() { return getSwag() != NULL; }
const SoundEvent* Llama::getAngrySound() { return SoundEvents::LLAMA_ANGRY; }
const SoundEvent* Llama::getAmbientSound() { return SoundEvents::LLAMA_AMBIENT; }
const SoundEvent* Llama::getHurtSound(const DamageSource*) { return SoundEvents::LLAMA_HURT; }
const SoundEvent* Llama::getDeathSound() { return SoundEvents::LLAMA_DEATH; }
void Llama::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::LLAMA_STEP, 0.15f, 1.0f);
}
void Llama::playChestEquipsSound() { AbstractChestedHorse::playChestEquipsSound(); }
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
bool Llama::isArmor(const not_null_ptr<ItemInstance>& item) { return !item->isEmpty() && item->getItem() == Item::byBlock(NULL); }
bool Llama::canBeSaddled() const { return false; }
unsigned int Llama::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData) { return AbstractHorse::GetInteractTooltip(tooltipData); }
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
void Llama::updateEquipment() { AbstractChestedHorse::updateEquipment(); }
int Llama::getMaxTemper() const { return 30; }
bool Llama::canMate(const boost::shared_ptr<Animal>& other) { return AbstractHorse::canMate(other); }
boost::shared_ptr<AgableMob> Llama::getBreedOffspring(boost::shared_ptr<AgableMob> partner) { return AbstractHorse::getBreedOffspring(partner); }
void Llama::setDidSpit(bool didSpit) { mDidSpit = didSpit; }
void Llama::causeFallDamage(float fallDistance, float damageMultiplier) { AbstractHorse::causeFallDamage(fallDistance, damageMultiplier); }
void Llama::leaveCaravan()
{
    if (mCaravanHead)
        mCaravanHead->mCaravanTail.reset();
    if (mCaravanTail)
        mCaravanTail->mCaravanHead.reset();
    mCaravanHead.reset();
    mCaravanTail.reset();
}

void Llama::joinCaravan(const boost::shared_ptr<Llama>& llama)
{
    if (!llama || llama.get() == this)
        return;
    mCaravanHead = llama;
    llama->mCaravanTail = boost::static_pointer_cast<Llama>(shared_from_this());
}
bool Llama::hasCaravanTail() { return mCaravanTail.get() != NULL; }
bool Llama::inCaravan() { return mCaravanHead.get() != NULL; }
boost::shared_ptr<Llama> Llama::getCaravanHead() { return mCaravanHead; }
double Llama::followLeashSpeed() { return 2.0; }
void Llama::followMommy()
{
    if (!inCaravan() && isBaby())
    {
        AbstractHorse::followMommy();
    }
}
bool Llama::canEatGrass() const { return false; }
unsigned long long Llama::GetType() { return 0x000000002D698003ULL; }
