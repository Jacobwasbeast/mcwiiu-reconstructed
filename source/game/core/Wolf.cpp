#include "game/core/Wolf.hpp"

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Llama.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

Wolf::Wolf(Level* level)
    : TamableAnimal(level), mBodyRollAngle(0.0f), mInterestedAngle(0),
      mIsInterested(0), mIsAngry(0), mHeadRollAngle(0.0f), mTailRollAngle(0.0f)
{
    setSize(0.6f, 0.85f);
}

Wolf::~Wolf() { }

void Wolf::create(Level* level) { new Wolf(level); }
void Wolf::addDataWalkers(DataFixerUpper* fixer) { Mob::addDataWalkers(fixer, 0xd6c0002); }
void Wolf::staticCtor() { }
void Wolf::registerGoals() { }
void Wolf::registerAttributes() { Mob::registerAttributes(); }
void Wolf::setAngry(bool angry) { mIsAngry = angry ? 1 : 0; }
void Wolf::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    setAngry(target != NULL && !isTame());
}
void Wolf::customServerAiStep() { }
void Wolf::defineSynchedData() { TamableAnimal::defineSynchedData(); }
void Wolf::playStepSound(const BlockPos&, Block*) { }
bool Wolf::isAngry() { return mIsAngry != 0; }
const DyeColor* Wolf::getCollarColor() { return DyeColor::RED; }
void Wolf::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Angry", isAngry());
    tag->putInt(L"CollarColor", getCollarColor()->getItemData());
}
void Wolf::setCollarColor(const DyeColor*) { }
void Wolf::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setAngry(tag->getBoolean(L"Angry"));
    if (tag->contains(L"CollarColor")) setCollarColor(DyeColor::byItemData(tag->getInt(L"CollarColor")));
}
const SoundEvent* Wolf::getAmbientSound() { return NULL; }
const SoundEvent* Wolf::getHurtSound(const DamageSource*) { return NULL; }
const SoundEvent* Wolf::getDeathSound() { return NULL; }
float Wolf::getSoundVolume() { return 1.0f; }
const void* Wolf::getDefaultLootTable() { return NULL; }
void Wolf::aiStep() { }
bool Wolf::isInterested() { return mIsInterested != 0; }
void Wolf::tick() { Mob::baseTick(); }
double Wolf::getBodyRollAngle(float partialTick, float scale)
{
    return mBodyRollAngle + (mHeadRollAngle - mBodyRollAngle) * partialTick * scale;
}
double Wolf::getHeadRollAngle(float partialTick) { return mHeadRollAngle * partialTick; }
double Wolf::getEyeHeight() { return mBbHeight * 0.8f; }
int Wolf::getMaxHeadXRot() { return 20; }
bool Wolf::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
bool Wolf::doHurtTarget(boost::shared_ptr<Entity> target) { return Mob::doHurtTarget(target); }
void Wolf::setTame(bool tame) { TamableAnimal::setTame(tame); }
void Wolf::tame(boost::shared_ptr<Player> player, bool success, bool showParticles)
{
    if (success) TamableAnimal::tame(player);
}
bool Wolf::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand) { return false; }
void Wolf::handleEntityEvent(unsigned char event, int data) { TamableAnimal::handleEntityEvent(event, data); }
double Wolf::getTailAngle() { return mTailRollAngle; }
bool Wolf::isFood(const not_null_ptr<ItemInstance>& item) { return !item->isEmpty(); }
int Wolf::getMaxSpawnClusterSize() { return 8; }
int Wolf::GetSynchedHealth() { return static_cast<int>(getHealth()); }
unsigned int Wolf::GetInteractTooltip(const EntityTooltipDataHolder&) { return 0; }
boost::shared_ptr<AgableMob> Wolf::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
void Wolf::setIsInterested(bool interested) { mIsInterested = interested ? 1 : 0; }
bool Wolf::canMate(const boost::shared_ptr<Animal>& other) { return Animal::canMate(other); }
bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target)
{
    return TamableAnimal::wantsToAttack(owner, target);
}
bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isAngry() && TamableAnimal::canBeLeashed(player); }
unsigned long long Wolf::GetType() { return 0x0d6c000200000000ULL; }
Wolf::WolfAvoidEntityGoal::WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType,
                                               float maxDistance, double walkSpeedModifier,
                                               double sprintSpeedModifier)
    : AvoidEntityGoal(wolf, avoidType, maxDistance, walkSpeedModifier, sprintSpeedModifier), mWolf(wolf) { }
Wolf::WolfAvoidEntityGoal::~WolfAvoidEntityGoal() { }
bool Wolf::WolfAvoidEntityGoal::avoidLlama(const boost::shared_ptr<Llama>& llama)
{
    return llama != NULL && mWolf != NULL && mWolf->getRandom()->nextInt(5) <= llama->getStrength();
}
bool Wolf::WolfAvoidEntityGoal::canUse() { return AvoidEntityGoal::canUse(); }
void Wolf::WolfAvoidEntityGoal::start() { AvoidEntityGoal::start(); }
void Wolf::WolfAvoidEntityGoal::tick() { AvoidEntityGoal::tick(); }
