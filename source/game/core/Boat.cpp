#include "game/core/Boat.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

Boat::Boat(Level* level) : Entity(level, true)
{
    _init();
}

Boat::Boat(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init();
    setPos(x, y, z);
}

Boat::~Boat()
{
}

void Boat::staticCtor()
{
}

void Boat::_init()
{
    mStatus = 0;
    mBubbleTime = 0;
    mHurtTime = 0;
    mHurtDir = 1;
    mDamage = 0.0f;
    mBubbleAngle = 0.0f;
    mBubbleAngleO = 0.0f;
    mLerpX = 0.0;
    mLerpY = 0.0;
    mLerpZ = 0.0;
    mLerpYRot = 0.0;
    mLerpXRot = 0.0;
    mLerpSteps = 0;
    mPaddleLeft = false;
    mPaddleRight = false;
    mInputLeft = false;
    mInputRight = false;
    mInputForward = false;
    mInputBackward = false;
    mOutOfControl = false;
    mSinking = false;
    mType = OAK;
    mBoundingBox = NULL;
    setSize(1.5f, 0.6f);
}

bool Boat::makeStepSound() { return false; }
void Boat::defineSynchedData() { }
AABB* Boat::getCollideAgainstBox(boost::shared_ptr<Entity> entity) { return entity.get() == this ? NULL : Entity::getCollideAgainstBox(entity); }
AABB* Boat::getCollideBox() { return mBoundingBox; }
bool Boat::isPushable(boost::shared_ptr<Entity> entity) { return entity.get() != this; }
double Boat::getRideHeight() { return 0.35; }
Boat::Type Boat::getType() { return mType; }
Item* Boat::getDropItem() { return NULL; }
bool Boat::hurt(const DamageSource* source, float damage) { setDamage(getDamage() + damage * 10.0f); return true; }
int Boat::getBubbleTime() { return mBubbleTime; }
void Boat::setBubbleTime(int time) { mBubbleTime = time; }
void Boat::onAboveBubbleCol(bool down) { mSinking = down; }
void Boat::push(boost::shared_ptr<Entity> entity) { Entity::push(entity); }
void Boat::animateHurt() { mHurtTime = 10; mHurtDir = -mHurtDir; }
bool Boat::isPickable() { return !mRemoved; }
void Boat::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x; mLerpY = y; mLerpZ = z; mLerpYRot = yRot; mLerpXRot = xRot; mLerpSteps = steps;
}
const Direction* Boat::getMotionDirection() { return Entity::getMotionDirection()->getClockWise(); }
bool Boat::isUnderwater() { return mStatus == 0; }
double Boat::getGroundFriction() { return 0.9; }
bool Boat::checkInWater() { return isInWater(); }
int Boat::getStatus() { return mStatus; }
const SoundEvent* Boat::getPaddleSound() { return NULL; }
bool Boat::getIsSinking() { return mSinking; }
void Boat::tickLerp() { if (mLerpSteps > 0) --mLerpSteps; }
void Boat::setPaddleState(bool left, bool right) { mPaddleLeft = left; mPaddleRight = right; }
double Boat::findBlockHeightOnPlane(const AABB* box, float& height, BlockPos& pos) { height = 0.0f; return height; }
double Boat::getWaterLevelAbove() { return mY; }
void Boat::floatBoat() { }
void Boat::controlBoat() { }
bool Boat::getPaddleState(int paddle) { return paddle == 0 ? mPaddleLeft : mPaddleRight; }
void Boat::setIsSinking(bool sinking) { mSinking = sinking; }
void Boat::tickBubbleColumn() { if (mBubbleTime > 0) --mBubbleTime; }
void Boat::tick() { Entity::tick(); tickLerp(); tickBubbleColumn(); }
double Boat::getRowingTime(int paddle, float partialTicks) { return getPaddleState(paddle) ? partialTicks : 0.0; }
void Boat::clampRotation(boost::shared_ptr<Entity> passenger) { }
void Boat::positionRider(const boost::shared_ptr<Entity>& passenger) { Entity::positionRider(passenger); }
void Boat::onPassengerTurned(boost::shared_ptr<Entity> passenger) { Entity::onPassengerTurned(passenger); }
void Boat::addAdditonalSaveData(CompoundTag* tag) { }
void Boat::setType(Type type) { mType = type; }
void Boat::readAdditionalSaveData(CompoundTag* tag) { }
bool Boat::testInteract(boost::shared_ptr<Player> player) { return getPassengers()->size() < 2; }
bool Boat::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand) { return testInteract(player); }
unsigned int Boat::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData) { return 0; }
unsigned int Boat::GetHitTooltip(const EntityTooltipDataHolder& tooltipData) { return 0; }
void Boat::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos) { }
void Boat::setDamage(float damage) { mDamage = damage; }
double Boat::getDamage() { return mDamage; }
void Boat::setHurtTime(int time) { mHurtTime = time; }
int Boat::getHurtTime() { return mHurtTime; }
double Boat::getBubbleAngle(float partialTicks) { return mBubbleAngleO + (mBubbleAngle - mBubbleAngleO) * partialTicks; }
void Boat::setHurtDir(int direction) { mHurtDir = direction; }
int Boat::getHurtDir() { return mHurtDir; }
bool Boat::canAddPassenger(boost::shared_ptr<Entity> entity) { return getPassengers()->size() < 2; }
boost::shared_ptr<Entity> Boat::getControllingPassenger() { return Entity::getControllingPassenger(); }
void Boat::setInput(bool left, bool right, bool forward, bool backward) { mInputLeft = left; mInputRight = right; mInputForward = forward; mInputBackward = backward; }
bool Boat::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isLeashed(); }
bool Boat::hasPlayerPassenger() { return getPassengers()->size() != 0; }
void Boat::tickLeash() { }
unsigned long long Boat::GetType() { return ENTITY_TYPE_BOAT; }
void Boat::create(Level* level) { new Boat(level); }
