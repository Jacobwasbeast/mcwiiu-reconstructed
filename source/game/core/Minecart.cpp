#include "game/core/Minecart.hpp"

#include <cmath>

#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/MinecartRideable.hpp"
#include "game/core/MinecartSpawner.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

const EntityDataAccessor<int>* Minecart::DATA_ID_DAMAGE = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_DISPLAY_BLOCK = NULL;
const EntityDataAccessor<float>* Minecart::DATA_ID_DISPLAY_OFFSET = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_HURT_TIME = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_HURT_DIR = NULL;
const EntityDataAccessor<bool>* Minecart::DATA_ID_CUSTOM_DISPLAY = NULL;

std::wstring Minecart::Type::NAMES[7] = {
    L"MinecartRideable",
    L"MinecartChest",
    L"MinecartFurnace",
    L"MinecartTNT",
    L"MinecartSpawner",
    L"MinecartHopper",
    L"MinecartCommandBlock"
};

const Minecart::Type* Minecart::Type::RIDEABLE = new Minecart::Type(0);
const Minecart::Type* Minecart::Type::CHEST = new Minecart::Type(1);
const Minecart::Type* Minecart::Type::FURNACE = new Minecart::Type(2);
const Minecart::Type* Minecart::Type::TNT = new Minecart::Type(3);
const Minecart::Type* Minecart::Type::SPAWNER = new Minecart::Type(4);
const Minecart::Type* Minecart::Type::HOPPER = new Minecart::Type(5);
const Minecart::Type* Minecart::Type::COMMAND_BLOCK = new Minecart::Type(6);

const Minecart::Type* Minecart::Type::VALUES[7] = {
    Minecart::Type::RIDEABLE,
    Minecart::Type::CHEST,
    Minecart::Type::FURNACE,
    Minecart::Type::TNT,
    Minecart::Type::SPAWNER,
    Minecart::Type::HOPPER,
    Minecart::Type::COMMAND_BLOCK
};

Minecart::Minecart(Level* level) : Entity(level, true)
{
    _init();
}

Minecart::Minecart(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init();
    setPos(x, y, z);
    mXOld = x;
    mYOld = y;
    mZOld = z;
    mXo = x;
    mYo = y;
    mZo = z;
}

Minecart::~Minecart()
{
}

void Minecart::staticCtor()
{
    DATA_ID_DAMAGE = SynchedEntityData::defineId<int>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
    DATA_ID_DISPLAY_BLOCK = SynchedEntityData::defineId<int>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
    DATA_ID_DISPLAY_OFFSET = SynchedEntityData::defineId<float>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
    DATA_ID_HURT_TIME = SynchedEntityData::defineId<int>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
    DATA_ID_HURT_DIR = SynchedEntityData::defineId<int>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
    DATA_ID_CUSTOM_DISPLAY = SynchedEntityData::defineId<bool>(0x0a800000ULL, ENTITY_TYPE_MINECART, NULL);
}

void Minecart::_init()
{
    mDamageDirection = 0;
    mCustomDisplay = 0;
    mDisplayOffset = 0;
    mDisplayState = 0;
    mLerpSteps = 0;
    mLerpX = 0.25;
    mLerpY = 0.25;
    mLerpZ = 0.25;
    mLerpXd = 0.25;
    mLerpYd = 0.25;
    mLerpZd = 0.25;
    mLerpYRot = 0.0f;
    mLerpXRot = 0.0f;
}

void Minecart::setPos(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
    if (mBoundingBox != NULL)
    {
        double halfWidth = mBbWidth * 0.5;
        mBoundingBox->set(x - halfWidth, y, z - halfWidth, x + halfWidth, y + mBbHeight, z + halfWidth);
    }
}

boost::shared_ptr<Minecart> Minecart::createMinecart(Level* level, double x, double y, double z, const Type* type)
{
    switch (type->mId)
    {
    case 1:
        return boost::shared_ptr<Minecart>(new MinecartChest(level, x, y, z));
    case 2:
        return boost::shared_ptr<Minecart>(new MinecartFurnace(level, x, y, z));
    case 3:
        return boost::shared_ptr<Minecart>(new MinecartTNT(level, x, y, z));
    case 4:
        return boost::shared_ptr<Minecart>(new MinecartSpawner(level, x, y, z));
    case 5:
        return boost::shared_ptr<Minecart>(new MinecartHopper(level, x, y, z));
    default:
        return boost::shared_ptr<Minecart>(new MinecartRideable(level, x, y, z));
    }
}

bool Minecart::makeStepSound()
{
    return false;
}

void Minecart::defineSynchedData()
{
    mEntityData->createDateItem(DATA_ID_DAMAGE, 0);
    mEntityData->createDateItem(DATA_ID_DISPLAY_BLOCK, 0);
    mEntityData->createDateItem(DATA_ID_DISPLAY_OFFSET, 0.0f);
    mEntityData->createDateItem(DATA_ID_HURT_TIME, 0);
    mEntityData->createDateItem(DATA_ID_HURT_DIR, 6);
    mEntityData->createDateItem(DATA_ID_CUSTOM_DISPLAY, false);
}

AABB* Minecart::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    if (entity)
    {
        return entity->getCollideBox();
    }
    return NULL;
}

AABB* Minecart::getCollideBox()
{
    return NULL;
}

AABB* Minecart::getBoundingBoxForCulling()
{
    if (!hasCustomDisplay())
    {
        return AABB::newTemp(mBoundingBox);
    }

    const int offset = getDisplayOffset();
    const double growAmount = (double)Mth::abs(offset) * 0.0625;
    return mBoundingBox->grow(growAmount);
}

bool Minecart::isPushable(boost::shared_ptr<Entity> entity)
{
    return !mRemoved && entity.get() != this;
}

double Minecart::getRideHeight()
{
    return mBbHeight * 0.5;
}

bool Minecart::hurt(const DamageSource* source, float damage)
{
    if (mRemoved || isInvulnerableTo(source))
    {
        return false;
    }
    animateHurt();
    return true;
}

void Minecart::destroy(const DamageSource* source)
{
    remove();
    if (!mLevel->getGameRules()->getBoolean(6))
    {
        return;
    }

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::minecart, 1));
    if (hasCustomName())
    {
        item->setHoverName(getCustomName());
    }
    spawnAtLocation(item, 0.25f);
}

void Minecart::animateHurt()
{
    mHurtMarked = true;
    setHurtTime(10);
}

bool Minecart::isPickable()
{
    return !mRemoved;
}

const Direction* Minecart::getMotionDirection()
{
    const Direction* direction = getDirection();
    if (mDamageDirection != 0)
    {
        return direction->getOpposite()->getClockWise();
    }
    return direction->getClockWise();
}

void Minecart::onAboveBubbleCol(bool down)
{
    if (down)
    {
        mYd += 0.007;
    }
}

bool Minecart::shouldTick()
{
    return true;
}

void Minecart::tick()
{
    baseTick();
    if (mLerpSteps > 0)
    {
        mX += (mLerpX - mX) / mLerpSteps;
        mY += (mLerpY - mY) / mLerpSteps;
        mZ += (mLerpZ - mZ) / mLerpSteps;
        --mLerpSteps;
    }
}

double Minecart::getMaxSpeed()
{
    return 0.4;
}

void Minecart::activateMinecart(int x, int y, int z, bool powered)
{
}

void Minecart::comeOffTrack()
{
    const double maxSpeed = getMaxSpeed();
    mXd = Mth::clamp(mXd, -maxSpeed, maxSpeed);
    mZd = Mth::clamp(mZd, -maxSpeed, maxSpeed);
    if (mOnGround)
    {
        mXd *= 0.5;
        mZd *= 0.5;
    }
    lerpMotion(mXd, mYd, mZd);
    if (!mOnGround && !mCustomDisplay)
    {
        mXd *= 0.949999988079071;
        mZd *= 0.949999988079071;
    }
}

void Minecart::moveAlongTrack(const BlockPos& pos, const BlockState* state)
{
    if (state != NULL)
    {
        mNoPhysics = false;
    }
}

void Minecart::applyNaturalSlowdown()
{
    double factor = isVehicle() ? 0.997 : 0.96;
    mXd *= factor;
    mZd *= factor;
}

Vec3* Minecart::getPosOffs(double x, double y, double z, double distance)
{
    return new Vec3(mX + x * distance, mY + y * distance, mZ + z * distance);
}

Vec3* Minecart::getPos(double x, double y, double z)
{
    return new Vec3(mX + x, mY + y, mZ + z);
}

void Minecart::addAdditonalSaveData(CompoundTag* tag)
{
    if (tag != NULL && hasCustomDisplay())
    {
        tag->putBoolean(L"CustomDisplayTile", true);
        tag->putInt(L"DisplayData", mDisplayState);
        tag->putInt(L"DisplayOffset", getDisplayOffset());
    }
}

void Minecart::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag != NULL && tag->getBoolean(L"CustomDisplayTile"))
    {
        if (tag->contains(L"DisplayData", 8))
        {
            mDisplayState = static_cast<unsigned char>(tag->getInt(L"DisplayData"));
        }
        setDisplayOffset(tag->getInt(L"DisplayOffset"));
        setCustomDisplay(true);
    }
}

void Minecart::push(boost::shared_ptr<Entity> entity)
{
    if (isPushable(entity))
    {
        Entity::push(entity);
    }
}

bool Minecart::isInterpolating() const
{
    return mLerpSteps > 0;
}

float Minecart::getLerpToYRot() const
{
    return mLerpYRot;
}

float Minecart::getLerpToXRot() const
{
    return mLerpXRot;
}

void Minecart::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x;
    mLerpY = y;
    mLerpZ = z;
    mLerpYRot = yRot;
    mLerpXRot = xRot;
    mLerpSteps = steps + 2;
}

void Minecart::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    mLerpXd = xd;
    mLerpYd = yd;
    mLerpZd = zd;
}

void Minecart::setDamage(float damage)
{
    mDamageDirection = static_cast<unsigned char>(damage);
}

float Minecart::getDamage()
{
    return mDamageDirection;
}

void Minecart::setHurtTime(int hurtTime)
{
    getEntityData()->set(DATA_ID_HURT_TIME, hurtTime, true);
}

int Minecart::getHurtTime()
{
    return getEntityData()->get(DATA_ID_HURT_TIME);
}

void Minecart::setHurtDir(int hurtDir)
{
    getEntityData()->set(DATA_ID_HURT_DIR, hurtDir, true);
}

int Minecart::getHurtDir()
{
    return getEntityData()->get(DATA_ID_HURT_DIR);
}

const BlockState* Minecart::getDisplayBlockState()
{
    return hasCustomDisplay() ? Block::stateByIdWithData(mDisplayState) : getDefaultDisplayBlockState();
}

const BlockState* Minecart::getDefaultDisplayBlockState()
{
    return NULL;
}

int Minecart::getDisplayOffset()
{
    return hasCustomDisplay() ? mDisplayOffset : getDefaultDisplayOffset();
}

int Minecart::getDefaultDisplayOffset()
{
    return 6;
}

void Minecart::setDisplayBlockState(const BlockState* state)
{
    mDisplayState = static_cast<unsigned char>(Block::getIdWithData(state));
    setCustomDisplay(true);
}

void Minecart::setDisplayOffset(int offset)
{
    mDisplayOffset = offset;
    setCustomDisplay(true);
}

bool Minecart::hasCustomDisplay()
{
    return getEntityData()->get(DATA_ID_CUSTOM_DISPLAY);
}

void Minecart::setCustomDisplay(bool customDisplay)
{
    getEntityData()->set(DATA_ID_CUSTOM_DISPLAY, customDisplay, true);
}

unsigned int Minecart::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? 0x5fb7a25aU : 0xffffffffU;
}

boost::shared_ptr<Entity> Minecart::getControllingPassenger()
{
    return boost::shared_ptr<Entity>();
}

unsigned long long Minecart::GetType()
{
    return ENTITY_TYPE_MINECART;
}

std::wstring Minecart::Type::getName() const
{
    return NAMES[mId];
}
