#include "game/core/Minecart.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

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
    if (type == NULL || type->mId == 0)
    {
        return boost::shared_ptr<Minecart>(new Minecart(level, x, y, z));
    }
    return boost::shared_ptr<Minecart>(new Minecart(level, x, y, z));
}

bool Minecart::makeStepSound()
{
    return false;
}

void Minecart::defineSynchedData()
{
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
    return mBoundingBox;
}

AABB* Minecart::getBoundingBoxForCulling()
{
    return mBoundingBox;
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
    return Entity::getDirection();
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
    return !mRemoved;
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
    mNoPhysics = true;
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
    mLerpSteps = hurtTime;
}

int Minecart::getHurtTime()
{
    return mLerpSteps;
}

void Minecart::setHurtDir(int hurtDir)
{
    mDisplayOffset = hurtDir;
}

int Minecart::getHurtDir()
{
    return mDisplayOffset;
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
    return mCustomDisplay != 0;
}

void Minecart::setCustomDisplay(bool customDisplay)
{
    mCustomDisplay = customDisplay ? 1 : 0;
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
    switch (mId)
    {
    case 0: return L"rideable";
    case 1: return L"chest";
    case 2: return L"furnace";
    case 3: return L"tnt";
    case 4: return L"spawner";
    case 5: return L"hopper";
    case 6: return L"command_block";
    default: return std::wstring();
    }
}
