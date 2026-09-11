#include "game/core/Boat.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

namespace
{
const EntityDataAccessor<int>* sBoatDataHurtDir = 0;

double& boatWaterLevel(Boat* boat)
{
    return *reinterpret_cast<double*>(reinterpret_cast<char*>(boat) + 0x398);
}

float& boatGroundFriction(Boat* boat)
{
    return *reinterpret_cast<float*>(reinterpret_cast<char*>(boat) + 0x3a0);
}

double& boatWaterScanBase(Boat* boat)
{
    return *reinterpret_cast<double*>(reinterpret_cast<char*>(boat) + 0x3b0);
}

unsigned char& boatBubbleColumnActive(Boat* boat)
{
    return *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(boat) + 0x3b8);
}

unsigned char& boatBubbleColumnDown(Boat* boat)
{
    return *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(boat) + 0x3b9);
}

int floorBlockCoord(double value)
{
    int block = Mth::floor(value);
    if (value < static_cast<double>(block))
        --block;
    return block;
}

int ceilBlockCoord(double value)
{
    int block = Mth::floor(value);
    if (static_cast<float>(block) < static_cast<float>(value))
        ++block;
    return block;
}

const BlockState* resolveBoatBlockState(Level* level, EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(tickCache, pos);
    if (state->getBlock() != Blocks::AIR)
        state = level->getBlockState(pos);
    return state;
}

bool isWaterState(const BlockState* state)
{
    return state != NULL && state->getMaterial() == Material::WATER;
}
}

const EntityDataAccessor<int>* Boat::DATA_STATUS = 0;
const EntityDataAccessor<int>* Boat::DATA_HURT_TIME = 0;
const EntityDataAccessor<float>* Boat::DATA_DAMAGE = 0;
const EntityDataAccessor<int>* Boat::DATA_TYPE = 0;
const EntityDataAccessor<bool>* Boat::DATA_PADDLE_LEFT = 0;
const EntityDataAccessor<bool>* Boat::DATA_PADDLE_RIGHT = 0;
const EntityDataAccessor<int>* Boat::DATA_BUBBLE_TIME = 0;
const EntityDataAccessor<bool>* Boat::DATA_SINKING = 0;

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
    DATA_STATUS = SynchedEntityData::defineId<int>(0x8000008ULL, 0x8000000ULL,
                                                    EntityDataSerializers::INT);
    DATA_HURT_TIME = SynchedEntityData::defineId<int>(0x8000008ULL, 0x8000000ULL,
                                                      EntityDataSerializers::INT);
    sBoatDataHurtDir = SynchedEntityData::defineId<int>(0x8000008ULL, 0x8000000ULL,
                                                         EntityDataSerializers::INT);
    DATA_DAMAGE = SynchedEntityData::defineId<float>(0x8000008ULL, 0x8000000ULL,
                                                      EntityDataSerializers::FLOAT);
    DATA_TYPE = SynchedEntityData::defineId<int>(0x8000008ULL, 0x8000000ULL,
                                                  EntityDataSerializers::INT);
    DATA_PADDLE_LEFT = SynchedEntityData::defineId<bool>(0x8000008ULL, 0x8000000ULL,
                                                          EntityDataSerializers::BOOLEAN);
    DATA_PADDLE_RIGHT = SynchedEntityData::defineId<bool>(0x8000008ULL, 0x8000000ULL,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_BUBBLE_TIME = SynchedEntityData::defineId<int>(0x8000008ULL, 0x8000000ULL,
                                                         EntityDataSerializers::INT);
    DATA_SINKING = SynchedEntityData::defineId<bool>(0x8000008ULL, 0x8000000ULL,
                                                      EntityDataSerializers::BOOLEAN);
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
void Boat::defineSynchedData()
{
    Entity::defineSynchedData();
    getEntityData()->createDateItem(DATA_STATUS, 0);
    getEntityData()->createDateItem(DATA_HURT_TIME, 0);
    getEntityData()->createDateItem(sBoatDataHurtDir, 1);
    getEntityData()->createDateItem(DATA_DAMAGE, 0.0f);
    getEntityData()->createDateItem(DATA_TYPE, 0);
    getEntityData()->createDateItem(DATA_PADDLE_LEFT, false);
    getEntityData()->createDateItem(DATA_PADDLE_RIGHT, false);
    getEntityData()->createDateItem(DATA_BUBBLE_TIME, 0);
    getEntityData()->createDateItem(DATA_SINKING, false);
}
AABB* Boat::getCollideAgainstBox(boost::shared_ptr<Entity> entity) { return entity.get() == this ? NULL : Entity::getCollideAgainstBox(entity); }
AABB* Boat::getCollideBox() { return mBoundingBox; }
bool Boat::isPushable(boost::shared_ptr<Entity> entity) { return entity.get() != this; }
double Boat::getRideHeight() { return 0.35; }
Boat::Type Boat::getType()
{
    return static_cast<Type>(getEntityData()->get(DATA_TYPE));
}
Item* Boat::getDropItem()
{
    switch (getType())
    {
    case SPRUCE:
        return Items::spruceBoat;
    case BIRCH:
        return Items::birchBoat;
    case JUNGLE:
        return Items::jungleBoat;
    case ACACIA:
        return Items::acaciaBoat;
    case DARK_OAK:
        return Items::darkOakBoat;
    case OAK:
    default:
        return Items::boat;
    }
}
bool Boat::hurt(const DamageSource* source, float damage) { setDamage(getDamage() + damage * 10.0f); return true; }
int Boat::getBubbleTime()
{
    return getEntityData()->get(DATA_BUBBLE_TIME);
}

void Boat::setBubbleTime(int time)
{
    getEntityData()->set(DATA_BUBBLE_TIME, time, true);
}
void Boat::onAboveBubbleCol(bool down)
{
    if (!mLevel->mIsClientSide)
    {
        boatBubbleColumnActive(this) = 1;
        boatBubbleColumnDown(this) = down ? 1 : 0;
        if (getBubbleTime() == 0)
            setBubbleTime(60);
    }

    if (mLevel->mRandom->nextInt(4) == 0)
    {
        mLevel->addParticle(PARTICLE_SPLASH,
                            mX + static_cast<double>(mRandom->nextFloat()),
                            mY + 0.6,
                            mZ + static_cast<double>(mRandom->nextFloat()),
                            0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}
void Boat::push(boost::shared_ptr<Entity> entity) { Entity::push(entity); }
void Boat::animateHurt() { mHurtTime = 10; mHurtDir = -mHurtDir; }
bool Boat::isPickable() { return !mRemoved; }
void Boat::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x; mLerpY = y; mLerpZ = z; mLerpYRot = yRot; mLerpXRot = xRot; mLerpSteps = steps;
}
const Direction* Boat::getMotionDirection() { return Entity::getMotionDirection()->getClockWise(); }
bool Boat::isUnderwater()
{
    AABB* box = mBoundingBox;
    const double waterThreshold = box->mMaxY + 0.001;
    const int minX = floorBlockCoord(box->mMinX);
    const int maxX = ceilBlockCoord(box->mMaxX);
    const int minY = floorBlockCoord(box->mMinY);
    const int maxY = ceilBlockCoord(waterThreshold);
    const int minZ = floorBlockCoord(box->mMinZ);
    const int maxZ = ceilBlockCoord(box->mMaxZ);
    bool foundWater = false;

    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            for (int y = minY; y < maxY; ++y)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = resolveBoatBlockState(mLevel, mTickCache, pos);
                if (!isWaterState(state))
                    continue;

                const double liquidHeight =
                    LiquidBlock::getLiquidHeight(state, mLevel, pos);
                if (waterThreshold >= liquidHeight)
                    continue;

                if (state->getBlock() == Blocks::bubble_column_segment)
                    return 2;

                foundWater = true;
            }
        }
    }

    return foundWater ? 1 : 3;
}

double Boat::getGroundFriction()
{
    AABB* sampleBox = AABB::newTemp(mBoundingBox->mMinX,
                                    mBoundingBox->mMinY - 0.001,
                                    mBoundingBox->mMinZ,
                                    mBoundingBox->mMaxX,
                                    mBoundingBox->mMaxY,
                                    mBoundingBox->mMaxZ);
    const int minX = floorBlockCoord(sampleBox->mMinX) - 1;
    const int maxX = ceilBlockCoord(sampleBox->mMaxX) + 1;
    const int minY = floorBlockCoord(sampleBox->mMinY) - 1;
    const int maxY = ceilBlockCoord(sampleBox->mMaxY) + 1;
    const int minZ = floorBlockCoord(sampleBox->mMinZ) - 1;
    const int maxZ = ceilBlockCoord(sampleBox->mMaxZ) + 1;
    std::vector<AABB*> collisions;
    double frictionSum = 0.6;
    unsigned int sampleCount = 0;

    for (int x = minX; x <= maxX; ++x)
    {
        const bool onXEdge = x == minX || x == maxX;
        for (int z = minZ; z <= maxZ; ++z)
        {
            const bool onZEdge = z == minZ || z == maxZ;
            if (onXEdge && onZEdge)
                continue;

            for (int y = minY; y <= maxY; ++y)
            {
                if ((onXEdge || onZEdge) && y != minY && y != maxY)
                    continue;

                BlockPos pos(x, y, z);
                const BlockState* state = mLevel->getBlockState(mTickCache, pos);
                collisions.clear();
                state->addCollissionAABBs(mLevel, pos, sampleBox, &collisions, shared_from_this(), false);
                if (collisions.empty())
                    continue;

                frictionSum += state->getBlock()->mFriction;
                ++sampleCount;
            }
        }
    }

    if (sampleCount == 0)
        return 0.6;

    return static_cast<double>(static_cast<float>(frictionSum / static_cast<double>(sampleCount)));
}

bool Boat::checkInWater()
{
    AABB* box = mBoundingBox;
    boatWaterLevel(this) = DBL_MAX;
    const int minX = floorBlockCoord(box->mMinX);
    const int maxX = ceilBlockCoord(box->mMaxX);
    const int minY = floorBlockCoord(box->mMinY);
    const int maxY = ceilBlockCoord(box->mMaxY + 0.001);
    const int minZ = floorBlockCoord(box->mMinZ);
    const int maxZ = ceilBlockCoord(box->mMaxZ);
    bool inWater = false;

    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            for (int y = minY; y < maxY; ++y)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = resolveBoatBlockState(mLevel, mTickCache, pos);
                if (!isWaterState(state))
                    continue;

                const double liquidHeight =
                    LiquidBlock::getLiquidHeight(state, mLevel, pos);
                if (liquidHeight < boatWaterLevel(this))
                    boatWaterLevel(this) = liquidHeight;
                inWater = inWater || box->mMinY < liquidHeight;
            }
        }
    }

    return inWater;
}
int Boat::getStatus()
{
    int status = isUnderwater();
    if (status != 3)
    {
        boatWaterLevel(this) =
            *reinterpret_cast<double*>(reinterpret_cast<char*>(mLevel) + 0x20);
        return status;
    }
    if (checkInWater())
        return 0;
    double friction = getGroundFriction();
    if (0.6 < friction)
    {
        boatGroundFriction(this) = static_cast<float>(friction);
        return 4;
    }
    return 5;
}
const SoundEvent* Boat::getPaddleSound()
{
    switch (getStatus())
    {
    case 0:
    case 1:
    case 2:
        return SoundEvents::BOAT_PADDLE_WATER;
    case 4:
        return SoundEvents::BOAT_PADDLE_LAND;
    default:
        return NULL;
    }
}
bool Boat::getIsSinking()
{
    return getEntityData()->get(DATA_SINKING);
}
void Boat::tickLerp() { if (mLerpSteps > 0) --mLerpSteps; }
void Boat::setPaddleState(bool left, bool right)
{
    getEntityData()->set(DATA_PADDLE_LEFT, left, true);
    getEntityData()->set(DATA_PADDLE_RIGHT, right, true);
}
double Boat::findBlockHeightOnPlane(const AABB* box, float& height, BlockPos& pos)
{
    const int minX = floorBlockCoord(box->mMinX);
    const int maxX = ceilBlockCoord(box->mMaxX);
    const int minZ = floorBlockCoord(box->mMinZ);
    const int maxZ = ceilBlockCoord(box->mMaxZ);
    const float threshold = 1.0f;

    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            pos = BlockPos(x, pos.y, z);
            const BlockState* state = resolveBoatBlockState(mLevel, mTickCache, pos);
            if (isWaterState(state))
            {
                const double liquidHeight = LiquidBlock::getLiquidHeight(state, mLevel, pos);
                if (static_cast<float>(static_cast<double>(height) - liquidHeight) < 0.0f)
                    height = static_cast<float>(liquidHeight);
            }

            if (threshold <= static_cast<double>(height))
                return 0.0;
        }
    }

    return 1.0;
}

double Boat::getWaterLevelAbove()
{
    AABB* box = mBoundingBox;
    const int minY = floorBlockCoord(box->mMaxY);
    const int maxY = ceilBlockCoord(box->mMaxY - boatWaterScanBase(this));
    const float initialHeight = 0.6f;
    BlockPos pos(0, 0, 0);

    for (int y = minY; y < maxY; ++y)
    {
        float height = initialHeight;
        pos = BlockPos(pos.x, y, pos.z);
        if (findBlockHeightOnPlane(box, height, pos) != 0.0 && height < 1.0f)
            return static_cast<double>(static_cast<float>(pos.y) + height);
    }

    return static_cast<double>(static_cast<float>(maxY) + initialHeight);
}
void Boat::floatBoat()
{
    const double water = getWaterLevelAbove();
    const double depth = water - mBoundingBox->mMinY;
    if (depth > 0.0)
    {
        const double buoyancy = depth > 1.0 ? 0.05 : depth * 0.05;
        mYd += buoyancy;
        mYd *= 0.9;
        mXd *= 0.9;
        mZd *= 0.9;
    }
    else
    {
        mYd -= 0.04;
        mXd *= 0.9;
        mZd *= 0.9;
    }
}
void Boat::controlBoat()
{
    boost::shared_ptr<Entity> passenger = getControllingPassenger();
    if (!passenger)
    {
        setPaddleState(false, false);
        return;
    }
    if (mInputLeft) mYRot -= 1.0f;
    if (mInputRight) mYRot += 1.0f;
    if (mInputForward)
    {
        mXd += (double)Mth::sin(-mYRot * 0.017453292f) * 0.04;
        mZd += (double)Mth::cos(mYRot * 0.017453292f) * 0.04;
    }
    setPaddleState(mInputForward || mInputLeft, mInputForward || mInputRight);
}
bool Boat::getPaddleState(int paddle) { return paddle == 0 ? mPaddleLeft : mPaddleRight; }
void Boat::setIsSinking(bool sinking)
{
    getEntityData()->set(DATA_SINKING, sinking, true);
}
void Boat::tickBubbleColumn() { if (mBubbleTime > 0) --mBubbleTime; }
void Boat::tick() { Entity::tick(); tickLerp(); tickBubbleColumn(); }
double Boat::getRowingTime(int paddle, float partialTicks) { return getPaddleState(paddle) ? partialTicks : 0.0; }
void Boat::clampRotation(boost::shared_ptr<Entity> passenger)
{
    if (!passenger)
        return;
    passenger->setRot(mYRot, passenger->getYHeadRot());
}
void Boat::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
        return;

    const double rideHeight = mOutOfControl ? 1.034999966621399 : getRideHeight();
    const double riderHeight = passenger->getRidingHeight();

    double lateralOffset = 0.0;
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (!passengers->empty() && passengers->size() > 1)
    {
        if (indexOf(*passengers, passenger) != 0)
            lateralOffset = -0.6000000238418579;
        if ((passenger->GetType() & 0x000000000D680000ULL) == 0x000000000D680000ULL)
            lateralOffset += 0.20000000298023224;
    }

    Vec3* riderOffset = Vec3::newTemp(lateralOffset, 0.0, 0.0);
    riderOffset = riderOffset->yRot(-(mYRot * 0.01745329238474369f - 1.5707963705062866f));

    passenger->setPos(mX + riderOffset->x, mY + rideHeight + riderHeight, mZ + riderOffset->z);

    const float rotDelta = mYRot - mYRotO;
    passenger->mYRot += rotDelta;
    passenger->setYHeadRot(passenger->getYHeadRot() + rotDelta);

    clampRotation(passenger);

    if ((passenger->GetType() & 0x000000000D680000ULL) == 0x000000000D680000ULL
        && !passengers->empty() && passengers->size() > 1)
    {
        const unsigned int bodyRotAngle = (passenger->getId() & 1) ? 270u : 90u;
        union
        {
            unsigned long long u;
            double d;
        } bodyRotPair;
        bodyRotPair.u = (static_cast<unsigned long long>(0x43300000u) << 32)
                      | static_cast<unsigned int>(bodyRotAngle ^ 0x80000000u);
        const float bodyRotOffset = static_cast<float>(bodyRotPair.d - 176.0);

        boost::shared_ptr<Animal> animal = boost::static_pointer_cast<Animal>(passenger);
        animal->setYBodyRot(animal->mYBodyRot + bodyRotOffset);
        passenger->setYHeadRot(passenger->getYHeadRot() + bodyRotOffset);
    }
}
void Boat::onPassengerTurned(boost::shared_ptr<Entity> passenger) { Entity::onPassengerTurned(passenger); }
void Boat::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Type", (int)mType);
}
void Boat::setType(Type type)
{
    getEntityData()->set(DATA_TYPE, static_cast<int>(type), true);
}
void Boat::readAdditionalSaveData(CompoundTag* tag)
{
    mType = (Type)Mth::clamp(tag->getInt(L"Type"), 0, 5);
}
bool Boat::testInteract(boost::shared_ptr<Player> player) { return getPassengers()->size() < 2; }
bool Boat::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand) { return testInteract(player); }
unsigned int Boat::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? 0x3c2d9ad6U : 0xffffffffU;
}
unsigned int Boat::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? 0x5fb7a25aU : 0xffffffffU;
}
void Boat::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    mFallDistance = (float)(mY - y);
    if (onGround && mFallDistance > 3.0f)
    {
        playSound(SoundEvents::BOAT_PADDLE_LAND, 1.0f, 1.0f);
        mFallDistance = 0.0f;
    }
    Entity::checkFallDamage(y, onGround, block, pos);
}
void Boat::setDamage(float damage)
{
    getEntityData()->set(DATA_DAMAGE, damage, true);
}

double Boat::getDamage()
{
    return static_cast<double>(getEntityData()->get(DATA_DAMAGE));
}

void Boat::setHurtTime(int time)
{
    getEntityData()->set(DATA_HURT_TIME, time, true);
}

int Boat::getHurtTime()
{
    return getEntityData()->get(DATA_HURT_TIME);
}
double Boat::getBubbleAngle(float partialTicks) { return mBubbleAngleO + (mBubbleAngle - mBubbleAngleO) * partialTicks; }
void Boat::setHurtDir(int direction)
{
    getEntityData()->set(sBoatDataHurtDir, direction, true);
}

int Boat::getHurtDir()
{
    return getEntityData()->get(sBoatDataHurtDir);
}
bool Boat::canAddPassenger(boost::shared_ptr<Entity> entity) { return getPassengers()->size() < 2; }
boost::shared_ptr<Entity> Boat::getControllingPassenger() { return Entity::getControllingPassenger(); }
void Boat::setInput(bool left, bool right, bool forward, bool backward) { mInputLeft = left; mInputRight = right; mInputForward = forward; mInputBackward = backward; }
bool Boat::canBeLeashed(const boost::shared_ptr<Player>& player) { return !isLeashed(); }
bool Boat::hasPlayerPassenger()
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    for (std::vector<boost::weak_ptr<Entity> >::const_iterator it = passengers->begin();
         it != passengers->end(); ++it)
    {
        boost::shared_ptr<Entity> passenger = it->lock();
        if (passenger && (passenger->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
            return true;
    }
    return false;
}
void Boat::tickLeash()
{
    if (!mLevel->mIsClientSide)
    {
        Entity::tickLeash();

        if (isLeashed())
        {
            boost::shared_ptr<Entity> holder = getLeashHolder();
            if (holder && holder->mLevel == mLevel)
            {
                const double distance = distanceTo(holder);
                if (distance > 10.0)
                {
                    dropLeash(true, true);
                }
                else if (distance > 0.0 && !hasPlayerPassenger())
                {
                    const double scale = static_cast<double>(static_cast<float>(0.6f / distance));
                    Vec3* boatPos = getPos(0.6f);
                    Vec3* holderPos = holder->getPos(0.6f);
                    const double dx = (holderPos->x - boatPos->x) * scale;
                    const double dy = (holderPos->y - boatPos->y) * scale;
                    const double dz = (holderPos->z - boatPos->z) * scale;
                    mXd += dx * std::fabs(dx) * 0.014;
                    mYd += dy * std::fabs(dy) * 0.014;
                    mZd += dz * std::fabs(dz) * 0.014;
                }
            }
        }
    }
}
unsigned long long Boat::GetType() { return ENTITY_TYPE_BOAT; }
void Boat::create(Level* level) { new Boat(level); }
