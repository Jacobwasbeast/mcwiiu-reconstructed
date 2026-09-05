#include "game/core/Llama.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/DyeColor.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

Llama::LlamaDefendTargetGoal::LlamaDefendTargetGoal(Llama* llama)
    : NearestAttackableTargetGoal(llama, typeid(LivingEntity), 10, true, false, NULL, false, false) { }
Llama::LlamaDefendTargetGoal::~LlamaDefendTargetGoal() { }
bool Llama::LlamaDefendTargetGoal::canUse() { return NearestAttackableTargetGoal::canUse(); }
double Llama::LlamaDefendTargetGoal::getFollowDistance() { return TargetGoal::getFollowDistance() * 0.25; }

Llama::LlamaGroupData::LlamaGroupData(int variant) : MobGroupData(), mVariant(variant) { }
Llama::LlamaGroupData::~LlamaGroupData() { }

Llama::LlamaHurtByTargetGoal::LlamaHurtByTargetGoal(Llama* llama)
    : HurtByTargetGoal(llama, false, arrayWithLength<unsigned long long>(NULL, 0)) { }
Llama::LlamaHurtByTargetGoal::~LlamaHurtByTargetGoal() { }
bool Llama::LlamaHurtByTargetGoal::canContinueToUse() { return HurtByTargetGoal::canUse(); }

Llama::Llama(Level* level)
    : AbstractChestedHorse(level), mDidSpit(false), mStrength(1), mVariant(0), mCaravanHead(), mCaravanTail() { }
Llama::~Llama() { }
void Llama::create(Level* level) { new Llama(level); }
const DyeColor* Llama::getSwag() { return DyeColor::WHITE; }
void Llama::containerChanged(Container* container) { AbstractChestedHorse::containerChanged(container); updateEquipment(); }
void Llama::spit(const boost::shared_ptr<LivingEntity>&) { mDidSpit = true; }
void Llama::performRangedAttack(boost::shared_ptr<LivingEntity> target, float) { spit(target); }
bool Llama::isAggressive() { return false; }
void Llama::setAggressive(bool) { }
void Llama::staticCtor() { }
void Llama::setStrength(int strength) { mStrength = strength < 1 ? 1 : (strength > 5 ? 5 : strength); }
void Llama::setRandomStrength() { setStrength(getRandom()->nextInt(3) + 1); }
int Llama::getStrength() { return mStrength; }
int Llama::getVariant() { return mVariant < 0 ? 0 : (mVariant > 3 ? 3 : mVariant); }
void Llama::addAdditonalSaveData(CompoundTag* tag) { AbstractChestedHorse::addAdditonalSaveData(tag); tag->putInt(L"Variant", getVariant()); tag->putInt(L"Strength", getStrength()); }
void Llama::setVariant(int variant) { mVariant = variant; }
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
void Llama::registerGoals() { AbstractHorse::registerGoals(); }
void Llama::registerAttributes() { AbstractChestedHorse::registerAttributes(); }
void Llama::defineSynchedData() { AbstractChestedHorse::defineSynchedData(); }
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
void Llama::playStepSound(const BlockPos&, Block*) { }
void Llama::playChestEquipsSound() { AbstractChestedHorse::playChestEquipsSound(); }
boost::shared_ptr<Entity> Llama::makeMad() { return boost::shared_ptr<Entity>(); }
const void* Llama::getDefaultLootTable() { return NULL; }
int Llama::getInventoryColumns() { return getStrength(); }
bool Llama::wearsArmor() { return true; }
bool Llama::isArmor(const not_null_ptr<ItemInstance>& item) { return !item->isEmpty() && item->getItem() == Item::byBlock(NULL); }
bool Llama::canBeSaddled() const { return false; }
unsigned int Llama::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData) { return AbstractHorse::GetInteractTooltip(tooltipData); }
bool Llama::HideUseTooltips() { return true; }
void Llama::setSwag(const DyeColor*) { }
void Llama::setSwagByItem(const not_null_ptr<ItemInstance>&) { }
void Llama::updateEquipment() { AbstractChestedHorse::updateEquipment(); }
int Llama::getMaxTemper() const { return 30; }
bool Llama::canMate(const boost::shared_ptr<Animal>& other) { return AbstractHorse::canMate(other); }
boost::shared_ptr<AgableMob> Llama::getBreedOffspring(boost::shared_ptr<AgableMob> partner) { return AbstractHorse::getBreedOffspring(partner); }
void Llama::setDidSpit(bool didSpit) { mDidSpit = didSpit; }
void Llama::causeFallDamage(float fallDistance, float damageMultiplier) { AbstractHorse::causeFallDamage(fallDistance, damageMultiplier); }
void Llama::leaveCaravan() { mCaravanHead.reset(); mCaravanTail.reset(); }
void Llama::joinCaravan(const boost::shared_ptr<Llama>& llama) { mCaravanHead = llama; }
bool Llama::hasCaravanTail() { return mCaravanTail.get() != NULL; }
bool Llama::inCaravan() { return mCaravanHead.get() != NULL; }
boost::shared_ptr<Llama> Llama::getCaravanHead() { return mCaravanHead; }
double Llama::followLeashSpeed() { return 2.0; }
boost::shared_ptr<Entity> Llama::followMommy() { return AbstractHorse::followMommy(); }
bool Llama::canEatGrass() const { return false; }
unsigned long long Llama::GetType() { return 0x2d69800300000000ULL; }
