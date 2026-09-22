#include "game/core/Boat.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/core/GameRules.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/core/Wolf.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

namespace
{
const EntityDataAccessor<int>* sBoatDataHurtDir = 0;

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
    DATA_STATUS = SynchedEntityData::defineId<int>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                    EntityDataSerializers::INT);
    DATA_HURT_TIME = SynchedEntityData::defineId<int>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                      EntityDataSerializers::INT);
    sBoatDataHurtDir = SynchedEntityData::defineId<int>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                         EntityDataSerializers::INT);
    DATA_DAMAGE = SynchedEntityData::defineId<float>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                      EntityDataSerializers::FLOAT);
    DATA_TYPE = SynchedEntityData::defineId<int>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                  EntityDataSerializers::INT);
    DATA_PADDLE_LEFT = SynchedEntityData::defineId<bool>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                          EntityDataSerializers::BOOLEAN);
    DATA_PADDLE_RIGHT = SynchedEntityData::defineId<bool>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_BUBBLE_TIME = SynchedEntityData::defineId<int>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                         EntityDataSerializers::INT);
    DATA_SINKING = SynchedEntityData::defineId<bool>(ENTITY_TYPE_BOAT, ENTITY_TYPE_ENTITY,
                                                      EntityDataSerializers::BOOLEAN);
}

void Boat::_init()
{
    mStatus = 0;
    mStatusO = 0;
    mUnderwaterTicks = 0;
    mBubbleTime = 0;
    mHurtTime = 0;
    mHurtDir = 1;
    mDamage = 0.0f;
    mBubbleAngle = 0.0f;
    mBubbleAngleO = 0.0f;
    mBubbleShakeRamp = 0.0f;
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
AABB* Boat::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    if (!entity->isPushable(shared_from_this()))
        return NULL;
    return entity->mBoundingBox;
}
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
bool Boat::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    setHurtDir(-getHurtDir());
    setHurtTime(10);
    setDamage(getDamage() + damage * 10.0f);
    markHurt();

    if (getDamage() > 40.0f && mLevel->getGameRules()->getBoolean(6))
        spawnAtLocation(getDropItem(), 1, 0.0f);

    remove();
    return true;
}
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
        mBubbleColumnActive = 1;
        mBubbleColumnDown = down ? 1 : 0;
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
void Boat::push(boost::shared_ptr<Entity> entity)
{
    if (entity->GetType() == ENTITY_TYPE_BOAT)
    {
        if (entity->mBoundingBox->mMinY < Entity::mBoundingBox->mMaxY)
            Entity::push(entity);
    }
    else
    {
        if (entity->mBoundingBox->mMinY <= Entity::mBoundingBox->mMinY)
            Entity::push(entity);
    }
}
void Boat::animateHurt()
{
    setHurtDir(-getHurtDir());
    setHurtTime(10);
    setDamage(static_cast<float>(getDamage() * 11.0));
}
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
    mWaterLevel = DBL_MAX;
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
                if (liquidHeight < mWaterLevel)
                    mWaterLevel = liquidHeight;
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
        mWaterLevel = static_cast<double>(mLevel->getSeaLevel());
        return status;
    }
    if (checkInWater())
        return 0;
    double friction = getGroundFriction();
    if (0.6 < friction)
    {
        mGroundFriction = static_cast<float>(friction);
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
void Boat::tickLerp()
{
    if (mLerpSteps > 0 && !isControlledByLocalInstance())
    {
        const double steps = static_cast<double>(mLerpSteps);
        --mLerpSteps;

        mYRot = mYRot + static_cast<float>(Mth::wrapDegrees(mLerpYRot - static_cast<double>(mYRot)) / steps);
        mXRot = static_cast<float>(static_cast<double>(mXRot) + (mLerpXRot - static_cast<double>(mXRot)) / steps);

        setPos(mX + (mLerpX - mX) / steps, mY + (mLerpY - mY) / steps, mZ + (mLerpZ - mZ) / steps);
        setRot(mYRot, mXRot);
    }
}
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
    const int maxY = ceilBlockCoord(box->mMaxY - mWaterScanBase);
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
    const double horizontalSpeed = Vec3::newTemp(mXd, 0.0, mZd)->length();
    double rockSpeed = horizontalSpeed * 30.0 + 1.0;
    if (mRandom->nextInt(30) == 0)
        rockSpeed *= 10.0;
    mRockPhase += 0.05 * rockSpeed;

    BlockPos base(mX, mY, mZ);
    Material* materialBase = mLevel->getBlockState(mTickCache, base)->getMaterial();
    Material* materialBaseExtra = mLevel->getExtraBlockState(base)->getMaterial();
    BlockPos aboveBase = base.above();
    Material* materialAbove = mLevel->getBlockState(mTickCache, aboveBase)->getMaterial();
    Material* materialAboveExtra = mLevel->getExtraBlockState(aboveBase)->getMaterial();

    bool inWater = false;

    if (materialBase == Material::WATER || materialBaseExtra == Material::WATER)
    {
        if (materialAbove == Material::AIR && !getIsSinking())
        {
            inWater = true;
            mXd *= 0.9;
            mZd *= 0.9;

            const double bobY = getWaterLevelAbove() - (double)mBbHeight + 0.2;
            setPos(mX, bobY + sin(mRockPhase) * 0.04, mZ);
            mYd = 0.0;
            return;
        }
        else if (materialAbove == Material::WATER || materialAboveExtra == Material::WATER)
        {
            inWater = true;
            mXd *= 0.9;
            mZd *= 0.9;
        }
    }
    else if (materialBase == Material::BUBBLE_COLUMN && materialAbove == Material::BUBBLE_COLUMN)
    {
        mXd *= 0.9;
        mZd *= 0.9;
    }
    else if (materialBase == Material::AIR || materialBase == Material::TOP_SNOW)
    {
        BlockPos below = base.below();
        Material* materialBelow = mLevel->getBlockState(mTickCache, below)->getMaterial();
        if (materialBelow != Material::WATER)
            materialBelow = mLevel->getExtraBlockState(below)->getMaterial();

        if (!mOnGround)
        {
            const float friction = (materialBelow == Material::WATER) ? 0.9f : 1.0f;
            mXd *= friction;
            mZd *= friction;
            mGroundFriction = friction;
        }
        else
        {
            Block* belowBlock = mLevel->getBlock(below);
            const float friction = belowBlock->mFriction;
            mXd *= friction;
            mZd *= friction;
            mGroundFriction = friction;
        }
    }
    else
    {
        mXd *= mGroundFriction;
        mZd *= mGroundFriction;
    }

    if (getIsSinking())
    {
        const double bobY = getWaterLevelAbove() - (double)mBbHeight + 0.2;
        setPos(mX, bobY + sin(mRockPhase) * 0.04, mZ);
    }
    else if (inWater)
    {
        mYd += 0.007;
    }
    else
    {
        mYd -= 0.04;
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
bool Boat::getPaddleState(int paddle)
{
    return getEntityData()->get(paddle == 0 ? DATA_PADDLE_LEFT : DATA_PADDLE_RIGHT) && getControllingPassenger();
}
void Boat::setIsSinking(bool sinking)
{
    getEntityData()->set(DATA_SINKING, sinking, true);
}
void Boat::tickBubbleColumn()
{
    if (!mLevel->mIsClientSide)
    {
        if (!mBubbleColumnActive)
            setBubbleTime(0);

        int time = getBubbleTime();
        if (time > 0)
        {
            --time;
            setBubbleTime(time);

            if (time < 1 && mBubbleColumnDown)
            {
                setBubbleTime(0);
                mZd -= 0.7;
                mBubbleTime = 60;
                setIsSinking(true);
            }

            mBubbleColumnActive = 0;
        }

        return;
    }

    if (getBubbleTime() > 0)
        mBubbleShakeRamp = Mth::clamp(mBubbleShakeRamp + 0.05f, 0.0f, 1.0f);
    else
        mBubbleShakeRamp = Mth::clamp(mBubbleShakeRamp - 0.1f, 0.0f, 1.0f);

    mBubbleAngleO = mBubbleAngle;
    mBubbleAngle = static_cast<float>(Mth::sin(0.5f * static_cast<float>(mLevel->getGameTime()))) * 10.0f * mBubbleShakeRamp;
}
void Boat::tick()
{
    mStatusO = mStatus;
    mStatus = getStatus();
    if (mStatus == 1 || mStatus == 2)
        ++mUnderwaterTicks;
    else
        mUnderwaterTicks = 0;

    if (!mLevel->mIsClientSide && (mUnderwaterTicks > 59 || getIsSinking()))
        ejectPassengers();

    if (getHurtTime() > 0)
        setHurtTime(getHurtTime() - 1);
    if (getDamage() > 0.0f)
        setDamage(getDamage() - 1.0f);

    mZo = mZ;
    mXo = mX;
    mYo = mY;

    Entity::tick();
    tickLerp();
    tickLeash();

    if (!isControlledByLocalInstance())
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }
    else
    {
        bool resetPaddles = true;
        std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
        if (!passengers->empty())
        {
            boost::shared_ptr<Entity> firstPassenger = (*passengers)[0].lock();
            if (firstPassenger && (firstPassenger->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
                resetPaddles = false;
        }

        if (resetPaddles)
            setPaddleState(false, false);

        floatBoat();

        if (mLevel->mIsClientSide)
            controlBoat();

        move(eMoverType_Self, mXd, mYd, mZd, false);
    }

    tickBubbleColumn();
    checkInsideBlocks();
}
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
        if ((passenger->GetType() & Animal::ENTITY_TYPE_ANIMAL) == Animal::ENTITY_TYPE_ANIMAL)
            lateralOffset += 0.20000000298023224;
    }

    Vec3* riderOffset = Vec3::newTemp(lateralOffset, 0.0, 0.0);
    riderOffset = riderOffset->yRot(-(mYRot * 0.01745329238474369f - 1.5707963705062866f));

    passenger->setPos(mX + riderOffset->x, mY + rideHeight + riderHeight, mZ + riderOffset->z);

    const float rotDelta = mYRot - mYRotO;
    passenger->mYRot += rotDelta;
    passenger->setYHeadRot(passenger->getYHeadRot() + rotDelta);

    clampRotation(passenger);

    if ((passenger->GetType() & Animal::ENTITY_TYPE_ANIMAL) == Animal::ENTITY_TYPE_ANIMAL
        && !passengers->empty() && passengers->size() > 1)
    {
        const unsigned int bodyRotAngle = (passenger->getId() & 1) ? 270u : 90u;
        const float bodyRotOffset = static_cast<float>(bodyRotAngle) - 176.0f;

        boost::shared_ptr<Animal> animal = boost::static_pointer_cast<Animal>(passenger);
        animal->setYBodyRot(animal->mYBodyRot + bodyRotOffset);
        passenger->setYHeadRot(passenger->getYHeadRot() + bodyRotOffset);
    }
}
void Boat::onPassengerTurned(boost::shared_ptr<Entity> passenger) { Entity::onPassengerTurned(passenger); }
void Boat::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Type", (int)getType());
    tag->putBoolean(L"Leashed", mLeashed);

    boost::shared_ptr<Entity> leashHolder = getLeashHolder();
    if (leashHolder)
    {
        CompoundTag* leashTag = new CompoundTag();
        if ((leashHolder->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
            leashTag->putString(L"UUID", leashHolder->getUUID());
        tag->put(L"Leash", leashTag);
    }
}
void Boat::setType(Type type)
{
    getEntityData()->set(DATA_TYPE, static_cast<int>(type), true);
}
void Boat::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"Type", 3))
        setType(static_cast<Type>(tag->getInt(L"Type")));

    mLeashed = tag->getBoolean(L"Leashed");

    if (mLeashed && tag->contains(L"Leash", 10))
        mLeashInfoTag = tag->getCompound(L"Leash")->copy();
}
bool Boat::testInteract(boost::shared_ptr<Player> player) { return getPassengers()->size() < 2; }
bool Boat::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);
    if (itemInHand->getItem() == Items::lead)
    {
        if (!mLevel->mIsClientSide && isLeashedToPlayer(player))
        {
            dropLeash(true, true);
            return true;
        }
        if (canBeLeashed(player))
        {
            setLeashedTo(player, true);
            itemInHand->shrink(1);
            return true;
        }
    }

    if (player->isSneaking())
        return false;

    if (!mLevel->mIsClientSide)
    {
        if (mUnderwaterTicks < 60 && !getIsSinking())
            player->startRiding(shared_from_this());

        if (isLeashedToPlayer(player))
        {
            dropLeash(true, true);
            return true;
        }
    }

    return true;
}
unsigned int Boat::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() == Items::lead)
    {
        if (!isLeashed())
            return Wolf::TOOLTIP_LEASH;

        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return Wolf::TOOLTIP_UNLEASH;
    }

    if (getPassengers()->size() < 2)
    {
        if (testInteract(tooltipData.m_pPlayer))
            return static_cast<unsigned int>(TOOLTIP_BOARD);
    }

    return 0xffffffffU;
}
unsigned int Boat::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? TOOLTIP_HIT_ALLOWED : 0xffffffffU;
}
void Boat::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    mWaterScanBase = mYd;

    if (isPassenger())
        return;

    if (block != NULL)
    {
        if (mFallDistance > 3.0f && mStatus == 4)
        {
            causeFallDamage(mFallDistance, 1.0f);
            if (!mLevel->mIsClientSide && !mRemoved)
            {
                remove();
                if (mLevel->getGameRules()->getBoolean(6))
                {
                    for (int i = 0; i < 3; ++i)
                        spawnAtLocation(new ItemInstance(Item::byBlock(Blocks::planks), 1, getType()), 0.0f);
                    spawnAtLocation(Items::stick, 1, 0.0f);
                    spawnAtLocation(Items::stick, 1, 0.0f);
                }
            }
        }
        mFallDistance = 0.0f;
    }
    else
    {
        BlockPos below = BlockPos(this).below();
        const BlockState* state = mLevel->getBlockState(mTickCache, below);
        if (state->getMaterial() != Material::WATER && y < 0.0)
            mFallDistance -= (float)y;
    }
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
boost::shared_ptr<Entity> Boat::getControllingPassenger()
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (passengers->empty())
        return boost::shared_ptr<Entity>();
    return passengers->at(0).lock();
}
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
