#include "game/entity/mob/AbstractHorse.hpp"

AbstractHorse::AbstractHorse(Level* level)
    : Animal(level), mFlags(0), mJumping(false), mStanding(false), mEating(false), mBred(false), mSaddled(false), mTamed(false), mTemper(0), mTailCounter(0), mJumpRidingScale(0.0f), mOwnerUUID(), mInventory(), mFloatGoal(0), mWaterAvoidingRandomStrollGoal(0)
{
}

AbstractHorse::~AbstractHorse()
{
}

bool AbstractHorse::getFlag(int flag) { return (mFlags & flag) != 0; }
bool AbstractHorse::isSaddled() { return mSaddled; }
void AbstractHorse::containerChanged(Container*) { }
void AbstractHorse::setFlag(int flag, bool value) { if (value) mFlags |= flag; else mFlags &= ~flag; }
void AbstractHorse::setEating(bool eating) { mEating = eating; }
void AbstractHorse::setStanding(bool standing) { mStanding = standing; }
void AbstractHorse::stand() { setStanding(true); }
void AbstractHorse::onPlayerJump(int) { }
bool AbstractHorse::canJump() { return true; }
double AbstractHorse::getJumpRidingScale() { return mJumpRidingScale; }
void AbstractHorse::setJumpRidingScale(float scale) { mJumpRidingScale = scale; }
void AbstractHorse::handleStartJump(int) { setIsJumping(true); }
void AbstractHorse::handleStopJump() { setIsJumping(false); }
std::wstring AbstractHorse::getOwnerUUID() { return mOwnerUUID; }
boost::shared_ptr<LivingEntity> AbstractHorse::getOwner() { return boost::shared_ptr<LivingEntity>(); }
int AbstractHorse::getTailCounter() const { return mTailCounter; }
void AbstractHorse::staticCtor() { }
void AbstractHorse::fjDerivedCtorCalls() { }
void AbstractHorse::registerGoals() { }
void AbstractHorse::defineSynchedData() { }
bool AbstractHorse::isTamed() { return mTamed; }
void AbstractHorse::setOwnerUUID(const std::wstring& uuid) { mOwnerUUID = uuid; }
double AbstractHorse::getFoalScale() const { return 1.0; }
void AbstractHorse::updateSize(bool) { }
bool AbstractHorse::isJumping() const { return mJumping; }
void AbstractHorse::setTamed(bool tamed) { mTamed = tamed; }
void AbstractHorse::setIsJumping(bool jumping) { mJumping = jumping; }
bool AbstractHorse::canBeLeashed(const boost::shared_ptr<Player>&) { return true; }
bool AbstractHorse::isEating() { return mEating; }
void AbstractHorse::onLeashDistance(float) { }
bool AbstractHorse::isStanding() { return mStanding; }
bool AbstractHorse::isBred() { return mBred; }
void AbstractHorse::setBred(bool bred) { mBred = bred; }
void AbstractHorse::setSaddled(bool saddled) { mSaddled = saddled; }
int AbstractHorse::getTemper() const { return mTemper; }
void AbstractHorse::setTemper(int temper) { mTemper = temper; }
int AbstractHorse::modifyTemper(int amount) { mTemper += amount; return mTemper; }
bool AbstractHorse::hurt(const DamageSource*, float) { return true; }
bool AbstractHorse::isPushable(boost::shared_ptr<Entity>) { return true; }
void AbstractHorse::openMouth() { setEating(true); }
void AbstractHorse::eating(int) { setEating(true); }
void AbstractHorse::causeFallDamage(float, float) { }
bool AbstractHorse::mobInteract(const boost::shared_ptr<Player>&, InteractionHand::EInteractionHand) { return false; }
int AbstractHorse::getInventorySize() { return 0; }
void AbstractHorse::createInventory() { }
void AbstractHorse::updateEquipment() { }
boost::shared_ptr<Entity> AbstractHorse::getClosestMommy(const boost::shared_ptr<Entity>&, double) { return boost::shared_ptr<Entity>(); }
double AbstractHorse::getCustomJump() { return 0.0; }
const SoundEvent* AbstractHorse::getJumpSound() { return 0; }
const SoundEvent* AbstractHorse::getDeathSound() { return 0; }
const SoundEvent* AbstractHorse::getHurtSound(const DamageSource*) { return 0; }
const SoundEvent* AbstractHorse::getAmbientSound() { return 0; }
const SoundEvent* AbstractHorse::getAngrySound() { return 0; }
bool AbstractHorse::canBeSaddled() const { return true; }
void AbstractHorse::playStepSound(const BlockPos&, Block*) { }
void AbstractHorse::playGallopSound(const SoundType*) { }
void AbstractHorse::registerAttributes() { }
int AbstractHorse::getMaxSpawnClusterSize() { return 4; }
int AbstractHorse::getMaxTemper() const { return 100; }
float AbstractHorse::getSoundVolume() { return 1.0f; }
int AbstractHorse::getAmbientSoundInterval() { return 120; }
void AbstractHorse::openInventory(const boost::shared_ptr<Player>&) { }
int AbstractHorse::CheckAndSetInLove(boost::shared_ptr<Player>) { return 0; }
int AbstractHorse::handleEating(const boost::shared_ptr<Player>&, const not_null_ptr<ItemInstance>&) { return 0; }
void AbstractHorse::doPlayerRide(const boost::shared_ptr<Player>&) { }
bool AbstractHorse::isImmobile() { return false; }
bool AbstractHorse::isFood(const not_null_ptr<ItemInstance>&) { return false; }
void AbstractHorse::moveTail() { ++mTailCounter; }
void AbstractHorse::die(const DamageSource*) { }
void AbstractHorse::aiStep() { }
boost::shared_ptr<Entity> AbstractHorse::followMommy() { return boost::shared_ptr<Entity>(); }
bool AbstractHorse::canEatGrass() const { return true; }
void AbstractHorse::tick() { }
boost::shared_ptr<Entity> AbstractHorse::makeMad() { return boost::shared_ptr<Entity>(); }
bool AbstractHorse::tameWithName(const boost::shared_ptr<Player>&) { return false; }
void AbstractHorse::travel(float, float, float) { }
void AbstractHorse::updateModelAnimation() { }
void AbstractHorse::addDataWalkers(DataFixerUpper*, unsigned long long) { }
void AbstractHorse::addAdditonalSaveData(CompoundTag*) { }
void AbstractHorse::readAdditionalSaveData(CompoundTag*) { }
bool AbstractHorse::canMate(const boost::shared_ptr<Animal>&) { return false; }
bool AbstractHorse::canParent() { return false; }
boost::shared_ptr<AgableMob> AbstractHorse::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
double AbstractHorse::generateRandomMaxHealth() { return 15.0; }
double AbstractHorse::generateRandomJumpStrength() { return 0.4; }
double AbstractHorse::generateRandomSpeed() { return 0.225; }
void AbstractHorse::setOffspringAttributes(const boost::shared_ptr<AgableMob>&, const boost::shared_ptr<AbstractHorse>&) { }
bool AbstractHorse::canBeControlledByRider() { return false; }
double AbstractHorse::getEatAnim(float) const { return 0.0; }
double AbstractHorse::getStandAnim(float) const { return 0.0; }
double AbstractHorse::getMouthAnim(float) const { return 0.0; }
void AbstractHorse::spawnTamingParticles(bool) { }
void AbstractHorse::handleEntityEvent(unsigned char, int) { }
void AbstractHorse::positionRider(const boost::shared_ptr<Entity>&) { }
bool AbstractHorse::onLadder() { return false; }
double AbstractHorse::getEyeHeight() { return 1.0; }
bool AbstractHorse::wearsArmor() { return false; }
bool AbstractHorse::isArmor(const not_null_ptr<ItemInstance>&) { return false; }
bool AbstractHorse::setSlot(int, const not_null_ptr<ItemInstance>&) { return false; }
boost::shared_ptr<Entity> AbstractHorse::getControllingPassenger() { return boost::shared_ptr<Entity>(); }
MobGroupData* AbstractHorse::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&) { return data; }
bool AbstractHorse::removeWhenFarAway() { return false; }
unsigned long long AbstractHorse::GetType() { return 0x0D69000000000000ULL; }
